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
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "WindowSystems/WaylandX11InputEvent.h"
#include "WindowSystems/WaylandX11WindowSystem.h"

#define ARRAY_LENGTH(a) (sizeof (a) / sizeof (a)[0])

extern "C"
{
    static int
    x11GetNextEvent(xcb_connection_t *conn, xcb_generic_event_t **event, uint32_t mask)
    {
        if (mask & WL_EVENT_READABLE){
            *event = xcb_poll_for_event(conn);
        } else {
#ifdef HAVE_XCB_POLL_FOR_QUEUED_EVENT
            *event = xcb_poll_for_queued_event(conn);
#else
            *event = xcb_poll_for_event(conn);
#endif
        }
        return *event != NULL;
    }

    static bool
    getButtonEvent(xcb_generic_event_t *event, WLEvent *wlEvent, int /*state*/)
    {
        xcb_button_press_event_t *buttonEvent = (xcb_button_press_event_t*)event;
        switch (buttonEvent->detail){
        default:
            wlEvent->button = buttonEvent->detail + BTN_LEFT - 1;
            break;
        case 2:
            wlEvent->button = BTN_MIDDLE;
            break;
        case 3:
            wlEvent->button = BTN_RIGHT;
            break;
        case 4:
        case 5:
        case 6:
        case 7:
            return false;
        }
        return true;
    }
}

/////////////////////////////////////////////////////////////////////////////

WaylandX11InputEvent::WaylandX11InputEvent(WaylandBaseWindowSystem *windowSystem)
: WaylandInputEvent(windowSystem)
, m_x11Display(NULL)
, m_xcbConn(NULL)
, m_hasXkb(false)
, m_xkbEventBase(0)
{
    memset(&m_xkbNames, 0x00, sizeof(m_xkbNames));
}

WaylandX11InputEvent::~WaylandX11InputEvent()
{
#if 0
    // WaylandX11WindowSystem should do this
    if (m_x11Display)
        XCloseDisplay(m_x11Display);
#endif
}

#define F(field) offsetof(struct atom_, field)

void
WaylandX11InputEvent::setupInputEvent()
{
    LOG_DEBUG("WaylandX11InputEvent", "setupInputEvent IN");

    WaylandInputEvent::setupInputEvent();

    WaylandX11WindowSystem* x11WindowSystem
        = dynamic_cast<WaylandX11WindowSystem*>(m_windowSystem);
    if (!x11WindowSystem){
        LOG_ERROR("WaylandX11InputEvent", "Invalid window system");
        return;
    }

    m_x11Display = x11WindowSystem->x11Display();
    if (!m_x11Display){
        LOG_ERROR("WaylandX11InputEvent", "ERROR: WindowSystem has not x11 display");
        return;
    }

    m_xcbConn = XGetXCBConnection(m_x11Display);
    XSetEventQueueOwner(m_x11Display, XCBOwnsEventQueue);
    if (xcb_connection_has_error(m_xcbConn)){
        LOG_ERROR("WaylandX11InputEvent", "ERROR: xcb connection has error.");
        XCloseDisplay(m_x11Display);
        return;
    }

    xcb_screen_iterator_t iter = xcb_setup_roots_iterator(xcb_get_setup(m_xcbConn));
    m_xcbScreen = iter.data;

    // Get resources
    {
        static const struct {
            const char *name;
            int offset;
        } atoms[] = {
            { "STRING",           F(string)   },
            { "_XKB_RULES_NAMES", F(xkbNames) }
        };
        xcb_intern_atom_cookie_t cookies[2];
        xcb_intern_atom_reply_t *reply;
        unsigned int i = 0;
        for (i = 0; i < ARRAY_LENGTH(atoms); ++i){
            cookies[i] = xcb_intern_atom(m_xcbConn, 0,
                                         strlen(atoms[i].name),
                                         atoms[i].name);
        }
        for (i = 0; i < ARRAY_LENGTH(atoms); ++i){
            reply = xcb_intern_atom_reply(m_xcbConn, cookies[i], NULL);
            *(xcb_atom_t*)((char *)(&m_atom) + atoms[i].offset) = reply->atom;
            free(reply);
        }
    }

    // Initialize pointer device
    initPointerDevice();

    // Initialize keyboard device
    setupXkb();
    struct xkb_keymap *keymap = getKeymap();
    initKeyboardDevice(keymap);
    if (keymap){
        xkb_map_unref(keymap);
    }

    // Regist event loop callback
    m_fd = xcb_get_file_descriptor(m_xcbConn);
    wl_event_loop *eventLoop = wl_display_get_event_loop(
        m_windowSystem->getNativeDisplayHandle());
    m_wlEventSource = wl_event_loop_add_fd(eventLoop,
                                           m_fd,
                                           WL_EVENT_READABLE,
                                           WaylandX11InputEvent::handleInputEvent,
                                           this);
    wl_event_source_check(m_wlEventSource);

    LOG_DEBUG("WaylandX11InputEvent", "setupInputEvent OUT");
}

void
WaylandX11InputEvent::setupXkb()
{
#ifndef HAVE_XCB_XKB
    return;
#else
    const xcb_query_extension_reply_t *ext;
    xcb_generic_error_t *error;
    xcb_void_cookie_t select;
    xcb_xkb_per_client_flags_cookie_t pcf;
    xcb_xkb_per_client_flags_reply_t *pcf_reply;

    ext = xcb_get_extension_data(m_xcbConn, &xcb_xkb_id);
    if (!ext){
        LOG_ERROR("WaylandX11InputEvent", "ERROR: XKB extension not available on "
                                          "host X11 server");
        return;
    }
    m_xkbEventBase = ext->first_event;

    select = xcb_xkb_select_events(m_xcbConn,
                                   XCB_XKB_ID_USE_CORE_KBD,
                                   XCB_XKB_EVENT_TYPE_STATE_NOTIFY,
                                   0,
                                   XCB_XKB_EVENT_TYPE_STATE_NOTIFY,
                                   0,
                                   0,
                                   NULL);
    error = xcb_request_check(m_xcbConn, select);
    if (error){
        LOG_ERROR("WaylandX11InputEvent", "ERROR: Failed to select XKB state events");
        return;
    }

    pcf = xcb_xkb_per_client_flags(m_xcbConn,
                                   XCB_XKB_ID_USE_CORE_KBD,
                                   XCB_XKB_PER_CLIENT_FLAG_DETECTABLE_AUTO_REPEAT,
                                   XCB_XKB_PER_CLIENT_FLAG_DETECTABLE_AUTO_REPEAT,
                                   0, 0, 0);
    pcf_reply = xcb_xkb_per_client_flags_reply(m_xcbConn, pcf, &error);
    free(pcf_reply);
    if (error){
        LOG_ERROR("WaylandX11InputEvent", "ERROR: Failed to set XKB per-client flags, "
                                          "not using detectable repeat");
        return;
    }

    m_hasXkb = true;
#endif
}

struct xkb_keymap*
WaylandX11InputEvent::getKeymap()
{
    xcb_get_property_cookie_t cookie;
    xcb_get_property_reply_t *reply;
    xcb_generic_error_t *error;
    struct xkb_rule_names names;
    struct xkb_keymap *ret;
    char *value_all, *value_part;
    int length_all, length_part;

    memset(&names, 0x00, sizeof(names));

    cookie = xcb_get_property(m_xcbConn,
                              0,
                              m_xcbScreen->root,
                              m_atom.xkbNames,
                              m_atom.string,
                              0, 1024);
    reply = xcb_get_property_reply(m_xcbConn, cookie, &error);
    if (reply == NULL){
        LOG_ERROR("WaylandX11InputEvent", "ERROR: xcb_get_property_reply");
        return NULL;
    }

    value_all  = static_cast<char*>(xcb_get_property_value(reply));
    length_all = xcb_get_property_value_length(reply);
    value_part = value_all;

#define copy_prop_value(to) \
    length_part = strlen(value_part); \
    if (value_part + length_part < (value_all + length_all) && \
        length_part > 0) \
        names.to = value_part; \
    value_part += length_part + 1;

    copy_prop_value(rules);
    copy_prop_value(model);
    copy_prop_value(layout);
    copy_prop_value(variant);
    copy_prop_value(options);
#undef copy_prop_value

    ret = xkb_map_new_from_names(m_xkbContext,
                                 &names,
                                 static_cast<xkb_map_compile_flags>(0));

    free(reply);
    return ret;
}

int
WaylandX11InputEvent::handleInputEvent(int /*fd*/, uint32_t mask, void *data)
{
    LOG_DEBUG("WaylandX11InputEvent", "handleInputEvent IN");

    WaylandX11InputEvent *ins = static_cast<WaylandX11InputEvent*>(data);
    WLEvent                    wlEvent;
    xcb_generic_event_t       *event;
    xcb_motion_notify_event_t *motionNotify;
    xcb_button_press_event_t  *buttonEvent;
    xcb_key_press_event_t     *keyPress, *keyRelease;
    InputDevice                deviceType = INPUT_DEVICE_POINTER;
    InputEventState            state      = INPUT_STATE_MOTION;

    int count = 0;
    xcb_generic_event_t *prev = NULL;
    while (x11GetNextEvent(ins->connection(), &event, mask)){
        switch (prev ? prev->response_type & ~0x80 : 0x80){
        case XCB_KEY_RELEASE:
            keyRelease = (xcb_key_press_event_t*)prev;
            keyPress   = (xcb_key_press_event_t*)event;
            if ((event->response_type & ~0x80) == XCB_KEY_PRESS &&
                keyRelease->time == keyPress->time &&
                keyRelease->detail == keyPress->detail){
                // Don't deliver the held key release event or
                // the new key press even.
                free(event);
                free(prev);
                prev = NULL;
                continue;
            } else {
                free(prev);
                prev = NULL;
                break;
            }
        case XCB_FOCUS_IN:
            free(prev);
            prev = NULL;
            break;
        default:
            // No previous event held
            break;
        }

        memset(&wlEvent, 0x00, sizeof(WLEvent));

        switch (event->response_type & ~0x80){
        case XCB_KEY_PRESS:
            LOG_DEBUG("WaylandX11InputEvent", "[EVENT] XCB_KEY_PRESS");
            keyPress = (xcb_key_press_event_t *)event;
            wlEvent.keyCode  = keyPress->detail - 8;
            wlEvent.keyState = WL_KEYBOARD_KEY_STATE_PRESSED;

            deviceType = INPUT_DEVICE_KEYBOARD;
            state      = INPUT_STATE_PRESSED;
            break;
        case XCB_KEY_RELEASE:
            LOG_DEBUG("WaylandX11InputEvent", "[EVENT] XCB_KEY_RELEASE");
            keyRelease = (xcb_key_press_event_t *)event;
            wlEvent.keyCode = keyRelease->detail - 8;
            wlEvent.keyState = WL_KEYBOARD_KEY_STATE_RELEASED;

            deviceType = INPUT_DEVICE_KEYBOARD;
            state      = INPUT_STATE_RELEASED;
            break;
        case XCB_BUTTON_PRESS:
            LOG_DEBUG("WaylandX11InputEvent", "[EVENT] XCB_BUTTON_PRESS");
            if (!getButtonEvent(event, &wlEvent, 1)){
                continue; // not supported button event
            }
            buttonEvent = (xcb_button_press_event_t *)event;
            wlEvent.x = buttonEvent->event_x;
            wlEvent.y = buttonEvent->event_y;
            wlEvent.buttonState = WL_POINTER_BUTTON_STATE_PRESSED;

            deviceType = INPUT_DEVICE_POINTER;
            state      = INPUT_STATE_PRESSED;
            break;
        case XCB_BUTTON_RELEASE:
            LOG_DEBUG("WaylandX11InputEvent", "[EVENT] XCB_BUTTON_RELEASE");
            if (!getButtonEvent(event, &wlEvent, 0)){
                continue; // not suported button event
            }
            buttonEvent = (xcb_button_press_event_t *)event;
            wlEvent.x = buttonEvent->event_x;
            wlEvent.y = buttonEvent->event_y;
            wlEvent.buttonState = WL_POINTER_BUTTON_STATE_RELEASED;

            deviceType = INPUT_DEVICE_POINTER;
            state      = INPUT_STATE_RELEASED;
            break;
        case XCB_MOTION_NOTIFY:
            LOG_DEBUG("WaylandX11InputEvent", "[EVENT] XCB_MOTION_NOTIFY");
            motionNotify = (xcb_motion_notify_event_t *)event;
            wlEvent.x = motionNotify->event_x;
            wlEvent.y = motionNotify->event_y;

            deviceType = INPUT_DEVICE_POINTER;
            state      = INPUT_STATE_MOTION;
            break;
        case XCB_EXPOSE:
        case XCB_ENTER_NOTIFY:
        case XCB_LEAVE_NOTIFY:
        case XCB_CLIENT_MESSAGE:
        case XCB_FOCUS_OUT:
        default:
            LOG_DEBUG("WaylandX11InputEvent", "[EVENT] Not supported (" <<
                      (event->response_type & ~0x80) << ")");
            if (prev != event){
                free(event);
            }
            continue;
        }

        ++count;
        if (prev != event){
            free(event);
        }
    }

    if (count == 0){
        goto rtn;
    }

    switch (prev ? prev->response_type & ~0x80 : 0x80){
    case XCB_KEY_RELEASE:
        keyRelease = (xcb_key_press_event_t*)prev;
        wlEvent.keyCode  = keyRelease->detail - 8;
        wlEvent.keyState = WL_KEYBOARD_KEY_STATE_RELEASED;
        deviceType = INPUT_DEVICE_KEYBOARD;
        state      = INPUT_STATE_RELEASED;
        free(prev);
        prev = NULL;
        break;
    default:
        break;
    }

    ins->windowSystem().manageWLInputEvent(deviceType, state, &wlEvent);

rtn:
    LOG_DEBUG("WaylandX11InputEvent", "handleInputEvent OUT");
    return count;
}
