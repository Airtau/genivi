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
#ifndef _WAYLANDINPUTDEVICE_H_
#define _WAYLANDINPUTDEVICE_H_
#include <wayland-server.h>
#include "Log.h"
#include "config.h"
#include "InputManager.h"

//////////////////////////////////////////////////////////////////////////////
class WaylandInputDevice
{
// Properties
private:
    struct wl_display* m_wlDisplay;
    struct wl_seat     m_wlSeat;
    bool               m_hasPointer;
    bool               m_hasKeyboard;
    bool               m_hasTouch;
    int                m_nTp;

    struct wl_surface*  m_wlTouchFocus;
    struct wl_listener  m_wlTouchFocusListener;
    struct wl_resource* m_wlTouchFocusResource;
    struct wl_listener  m_wlTouchFocusResourceListener;

    struct {
        struct wl_pointer  wlPointer;
        struct wl_keyboard wlKeyboard;
        struct wl_touch    wlTouch;
    } m_deviceInterfaces;

// Methods
public:
    WaylandInputDevice(wl_display *display);
    virtual ~WaylandInputDevice();

    void initPointerDevice();
    void initKeyboardDevice();
    void initTouchDevice();

    struct wl_display* display();
    struct wl_seat* seat();
    bool hasPointer() const;
    bool hasKeyboard() const;
    bool hasTouch() const;

    struct wl_pointer  *pointerDevice();
    struct wl_keyboard *keyboardDevice();
    struct wl_touch    *touchDevice();
    const struct wl_pointer  *pointerDevice() const;
    const struct wl_keyboard *keyboardDevice() const;
    const struct wl_touch    *touchDevice() const;

    void sendMousePressEvent(const Point& globalPos, const Point& localPos,
                             uint32_t button, uint32_t time);
    void sendMouseReleaseEvent(const Point& globalPos, const Point& localPos,
                               uint32_t button, uint32_t time);
    void sendMouseMotionEvent(struct wl_surface *surface,
                              const Point& globalPos, const Point& localPos, uint32_t time);

    void sendKeyPressEvent(struct wl_surface *surface, uint32_t time, uint32_t code);
    void sendKeyReleaseEvent(struct wl_surface *surface, uint32_t time, uint32_t code);
    void setKeyboardFocus(struct wl_surface *surface);

    void sendTouchPointEvent(struct wl_surface *surface, uint32_t time,
                             int touchId, int touchState, const Point& touchPos);
    void sendTouchFrameEvent();
    void sendTouchCancelEvent();

    void sendModifiers(uint32_t serial);

private:
    void releaseDevices();
    void cleanupDataDeviceForClient(struct wl_client *client, bool destroyDev);

    void setMouseFocus(struct wl_surface *surface,
                       const Point& globalPos,
                       const Point& localPos);
    void sendMouseMotionEvent(const Point& globalPos,
                              const Point& localPos,
                              uint32_t time);

    void setTouchFocus(struct wl_surface *surface);

    const static struct wl_seat_interface m_seatInterface;

    static void bindInputDevice(struct wl_client *client,
                                void *data,
                                uint32_t version,
                                uint32_t id);
    static void destroyResource(struct wl_resource *resource);
    static void destroyDeviceResource(struct wl_resource *resource);

    static void getPointer(struct wl_client *client,
                           struct wl_resource *resource,
                           uint32_t id);
    static void getKeyboard(struct wl_client *client,
                            struct wl_resource *resource,
                            uint32_t id);
    static void getTouch(struct wl_client *client,
                         struct wl_resource *resource,
                         uint32_t id);
};

inline struct wl_display* WaylandInputDevice::display() { return m_wlDisplay; }
inline struct wl_seat* WaylandInputDevice::seat() { return &m_wlSeat; }
inline bool WaylandInputDevice::hasPointer() const { return m_hasPointer; }
inline bool WaylandInputDevice::hasKeyboard() const { return m_hasKeyboard; }
inline bool WaylandInputDevice::hasTouch() const { return m_hasTouch; }

inline struct wl_pointer* WaylandInputDevice::pointerDevice()
    { return &m_deviceInterfaces.wlPointer; }
inline struct wl_keyboard* WaylandInputDevice::keyboardDevice()
    { return &m_deviceInterfaces.wlKeyboard; }
inline struct wl_touch* WaylandInputDevice::touchDevice()
    { return &m_deviceInterfaces.wlTouch; }

inline const struct wl_pointer* WaylandInputDevice::pointerDevice() const
    { return &m_deviceInterfaces.wlPointer; }
inline const struct wl_keyboard* WaylandInputDevice::keyboardDevice() const
    { return &m_deviceInterfaces.wlKeyboard; }
inline const struct wl_touch* WaylandInputDevice::touchDevice() const
    { return &m_deviceInterfaces.wlTouch; }

#endif /* _WAYLANDINPUTDEVICE_H_ */
