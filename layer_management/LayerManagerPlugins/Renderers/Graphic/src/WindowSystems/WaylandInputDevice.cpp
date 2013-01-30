/***************************************************************************
*
* Copyright 2010, 2011 BMW Car IT GmbH
* Copyright (C) 2011 DENSO CORPORATION and Robert Bosch Car Multimedia Gmbh
*
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*        http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*
* THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS
* SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
* FITNESS, IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY
* SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
* RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
* CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
* CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*
****************************************************************************/
#include <time.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "WindowSystems/WaylandInputDevice.h"

/////////////////////////////////////////////////////////////////////////////

static struct wl_resource*
findResourceForClient(struct wl_list *list, struct wl_client *client)
{
    struct wl_resource *r;
    wl_list_for_each(r, list, link){
        if (r->client == client)
            return r;
    }
    return NULL;
}

/////////////////////////////////////////////////////////////////////////////

WaylandInputDevice::WaylandInputDevice(struct wl_display *display)
: m_wlDisplay(display)
, m_hasPointer(false)
, m_hasKeyboard(false)
, m_hasTouch(false)
, m_nTp(0)
{
    wl_seat_init(&m_wlSeat);
    wl_display_add_global(m_wlDisplay,
                          &wl_seat_interface,
                          this,
                          WaylandInputDevice::bindInputDevice);
}

WaylandInputDevice::~WaylandInputDevice()
{
    releaseDevices();
}

void
WaylandInputDevice::initPointerDevice()
{
    if (m_hasPointer)
        return;

    wl_pointer_init(&m_deviceInterfaces.wlPointer);
    wl_seat_set_pointer(&m_wlSeat, &m_deviceInterfaces.wlPointer);

    m_hasPointer = true;
    LOG_INFO("WaylandInputDevice", "Available pointer");
}

void
WaylandInputDevice::initKeyboardDevice()
{
    if (m_hasKeyboard)
        return;

    wl_keyboard_init(&m_deviceInterfaces.wlKeyboard);
    wl_seat_set_keyboard(&m_wlSeat, &m_deviceInterfaces.wlKeyboard);

    m_hasKeyboard = true;
    LOG_INFO("WaylandInputDevice", "Available keyboard");
}

void
WaylandInputDevice::initTouchDevice()
{
    if (m_hasTouch)
        return;

    wl_touch_init(&m_deviceInterfaces.wlTouch);
    wl_seat_set_touch(&m_wlSeat, &m_deviceInterfaces.wlTouch);

    m_hasTouch = true;
    LOG_INFO("WaylandInputDevice", "Available touch");
}

void
WaylandInputDevice::releaseDevices()
{
    if (m_hasPointer)
        wl_pointer_release(&m_deviceInterfaces.wlPointer);

    if (m_hasKeyboard)
        wl_keyboard_release(&m_deviceInterfaces.wlKeyboard);

    if (m_hasTouch)
        wl_touch_release(&m_deviceInterfaces.wlTouch);
}

void
WaylandInputDevice::cleanupDataDeviceForClient(struct wl_client *client, bool destroyDev)
{
    (void)client;
    (void)destroyDev;
}

void
WaylandInputDevice::bindInputDevice(struct wl_client *client,
                                    void *data,
                                    uint32_t /*version*/,
                                    uint32_t id)
{
    struct wl_resource *resource =
        wl_client_add_object(client,
                             &wl_seat_interface,
                             &WaylandInputDevice::m_seatInterface,
                             id,
                             data);

    WaylandInputDevice *inputDevice = static_cast<WaylandInputDevice*>(data);
    struct wl_seat *seat = inputDevice->seat();

    resource->destroy = WaylandInputDevice::destroyResource;
    wl_list_insert(&seat->base_resource_list, &resource->link);

    uint32_t caps = 0;

    if (inputDevice->hasPointer())
        caps |= WL_SEAT_CAPABILITY_POINTER;
    if (inputDevice->hasKeyboard())
        caps |= WL_SEAT_CAPABILITY_KEYBOARD;
    if (inputDevice->hasTouch())
        caps |= WL_SEAT_CAPABILITY_TOUCH;

    wl_seat_send_capabilities(resource, caps);
}

void
WaylandInputDevice::destroyResource(struct wl_resource *resource)
{
    WaylandInputDevice *inputDevice = static_cast<WaylandInputDevice*>(resource->data);
    if (inputDevice->keyboardDevice()->focus_resource == resource){
        inputDevice->keyboardDevice()->focus_resource = 0;
    }
    if (inputDevice->pointerDevice()->focus_resource == resource){
        inputDevice->pointerDevice()->focus_resource = 0;
    }

    inputDevice->cleanupDataDeviceForClient(resource->client, true);

    wl_list_remove(&resource->link);

    free(resource);
}

void
WaylandInputDevice::destroyDeviceResource(struct wl_resource *resource)
{
    wl_list_remove(&resource->link);
    free(resource);
}

const struct wl_seat_interface WaylandInputDevice::m_seatInterface = {
    getPointer,
    getKeyboard,
    getTouch
};

void
WaylandInputDevice::getPointer(struct wl_client *client,
                               struct wl_resource *resource,
                               uint32_t id)
{
    WaylandInputDevice *inputDevice = static_cast<WaylandInputDevice*>(resource->data);
    wl_pointer *pointer = inputDevice->pointerDevice();
    wl_resource *clientResource = wl_client_add_object(client,
                                                       &wl_pointer_interface,
                                                       0,
                                                       id,
                                                       pointer);
    wl_list_insert(&pointer->resource_list, &clientResource->link);
    clientResource->destroy = WaylandInputDevice::destroyDeviceResource;
}

void
WaylandInputDevice::getKeyboard(struct wl_client *client,
                                struct wl_resource *resource,
                                uint32_t id)
{
    WaylandInputDevice *inputDevice = static_cast<WaylandInputDevice*>(resource->data);
    wl_keyboard *keyboard = inputDevice->keyboardDevice();
    wl_resource *clientResource = wl_client_add_object(client,
                                                       &wl_keyboard_interface,
                                                       0,
                                                       id,
                                                       keyboard);
    wl_list_insert(&keyboard->resource_list, &clientResource->link);
    clientResource->destroy = WaylandInputDevice::destroyDeviceResource;
}

void
WaylandInputDevice::getTouch(struct wl_client *client,
                             struct wl_resource *resource,
                             uint32_t id)
{
    WaylandInputDevice *inputDevice = static_cast<WaylandInputDevice*>(resource->data);
    wl_touch *touch = inputDevice->touchDevice();
    wl_resource *clientResource = wl_client_add_object(client,
                                                       &wl_touch_interface,
                                                       0,
                                                       id,
                                                       touch);
    wl_list_insert(&touch->resource_list, &clientResource->link);
    clientResource->destroy = WaylandInputDevice::destroyDeviceResource;
}

void
WaylandInputDevice::sendMousePressEvent(const Point& globalPos,
                                        const Point& localPos,
                                        uint32_t button,
                                        uint32_t time)
{
    sendMouseMotionEvent(globalPos, localPos, time);
    wl_pointer *pointer = pointerDevice();
    pointer->button_count++;
    const struct wl_pointer_grab_interface *interface = pointer->grab->interface;
    interface->button(pointer->grab,
                      time,
                      button,
                      WL_POINTER_BUTTON_STATE_PRESSED);
}

void
WaylandInputDevice::sendMouseReleaseEvent(const Point& globalPos,
                                          const Point& localPos,
                                          uint32_t button,
                                          uint32_t time)
{
    sendMouseMotionEvent(globalPos, localPos, time);
    wl_pointer *pointer = pointerDevice();
    pointer->button_count--;
    const struct wl_pointer_grab_interface *interface = pointer->grab->interface;
    interface->button(pointer->grab,
                      time,
                      button,
                      WL_POINTER_BUTTON_STATE_RELEASED);
}

void
WaylandInputDevice::sendMouseMotionEvent(struct wl_surface* surface,
                                         const Point& globalPos,
                                         const Point& localPos,
                                         uint32_t time)
{
    setMouseFocus(surface, globalPos, localPos);
    sendMouseMotionEvent(globalPos, localPos, time);
}

void
WaylandInputDevice::sendMouseMotionEvent(const Point& globalPos,
                                         const Point& localPos,
                                         uint32_t time)
{
    wl_pointer *pointer = pointerDevice();
    const struct wl_pointer_grab_interface *interface = pointer->grab->interface;
    pointer->x = wl_fixed_from_double(globalPos.x);
    pointer->y = wl_fixed_from_double(globalPos.y);
    interface->motion(pointer->grab,
                      time,
                      wl_fixed_from_double(localPos.x),
                      wl_fixed_from_double(localPos.y));
}

void
WaylandInputDevice::sendKeyPressEvent(struct wl_surface* surface,
                                      uint32_t time, uint32_t code)
{
    wl_keyboard *keyboard = keyboardDevice();
    if (!keyboard->focus){
        setKeyboardFocus(surface);
    }
    if (keyboard->focus_resource){
        uint32_t serial = wl_display_next_serial(m_wlDisplay);
        wl_keyboard_send_key(keyboard->focus_resource,
                             serial, time, code, 1);
    }
}

void
WaylandInputDevice::sendKeyReleaseEvent(struct wl_surface* /*surface*/,
                                        uint32_t time, uint32_t code)
{
    wl_keyboard *keyboard = keyboardDevice();
    if (keyboard->focus_resource){
        uint32_t serial = wl_display_next_serial(m_wlDisplay);
        wl_keyboard_send_key(keyboard->focus_resource,
                             serial, time, code, 0);
    }
}

void
WaylandInputDevice::sendTouchPointEvent(struct wl_surface* surface, uint32_t time,
                                        int touchId, int touchState, const Point& touchPos)
{
    switch (touchState){
    case WL_TOUCH_DOWN:
        ++m_nTp;
        if (m_nTp == 1){
            setTouchFocus(surface);
        }
        if (m_wlTouchFocusResource && m_wlTouchFocus){
            wl_touch_send_down(m_wlTouchFocusResource,
                               0 /*serial*/, time,
                               &m_wlTouchFocus->resource, touchId,
                               wl_fixed_from_double(touchPos.x),
                               wl_fixed_from_double(touchPos.y));
        }
        break;
    case WL_TOUCH_MOTION:
        if (!m_wlTouchFocus){
            break;
        }
        if (m_wlTouchFocusResource){
            wl_touch_send_motion(m_wlTouchFocusResource,
                                 time, touchId,
                                 wl_fixed_from_double(touchPos.x),
                                 wl_fixed_from_double(touchPos.y));
        }
        break;
    case WL_TOUCH_UP:
        --m_nTp;
        if (m_wlTouchFocusResource){
            wl_touch_send_up(m_wlTouchFocusResource, 0 /*serial*/, time, touchId);
        }
        if (m_nTp == 0){
            setTouchFocus(NULL);
        }
        break;
    }
}

void
WaylandInputDevice::sendTouchFrameEvent()
{
    wl_touch *touch = touchDevice();
    wl_resource *resource = touch->focus_resource;
    if (resource){
        wl_touch_send_frame(resource);
    }
}

void
WaylandInputDevice::sendTouchCancelEvent()
{
    wl_touch *touch = touchDevice();
    wl_resource *resource = touch->focus_resource;
    if (resource){
        wl_touch_send_cancel(resource);
    }
}

void
WaylandInputDevice::setMouseFocus(struct wl_surface* surface,
                                  const Point& globalPos,
                                  const Point& localPos)
{
    wl_pointer* pointer = pointerDevice();
    pointer->x = wl_fixed_from_double(globalPos.x);
    pointer->y = wl_fixed_from_double(globalPos.y);
    pointer->current = surface;
    pointer->current_x = wl_fixed_from_double(localPos.x);
    pointer->current_y = wl_fixed_from_double(localPos.y);
    pointer->grab->interface->focus(pointer->grab,
                                    surface,
                                    wl_fixed_from_double(localPos.x),
                                    wl_fixed_from_double(localPos.y));
}

void
WaylandInputDevice::setKeyboardFocus(struct wl_surface* surface)
{
    wl_keyboard_set_focus(keyboardDevice(), surface);
}

void
WaylandInputDevice::setTouchFocus(struct wl_surface* surface)
{
    struct wl_resource* resource = NULL;

    if (m_wlTouchFocus == surface)
        return;

    if (surface){
        resource = findResourceForClient(&m_wlSeat.touch->resource_list,
                                         surface->resource.client);
        if (!resource){
            return;
        }
        m_wlSeat.touch->focus = surface;
        m_wlSeat.touch->focus_resource = resource;
    }
    else {
        m_wlSeat.touch->focus = NULL;
        m_wlSeat.touch->focus_resource = NULL;
    }
}

void
WaylandInputDevice::sendModifiers(uint32_t serial)
{
    struct wl_keyboard *keyboard = keyboardDevice();
    struct wl_keyboard_grab *grab = keyboard->grab;
    grab->interface->modifiers(grab, serial,
                               keyboard->modifiers.mods_depressed,
                               keyboard->modifiers.mods_latched,
                               keyboard->modifiers.mods_locked,
                               keyboard->modifiers.group);
}
