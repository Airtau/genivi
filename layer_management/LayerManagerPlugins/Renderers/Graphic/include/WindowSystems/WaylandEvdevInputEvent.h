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

#ifndef _WAYLANDEVDEVINPUTEVENT_H_
#define _WAYLANDEVDEVINPUTEVENT_H_

#include "config.h"
#include <wayland-server.h>
#include "Log.h"
#include "WindowSystems/WaylandInputEvent.h"
#include "WindowSystems/WaylandBaseWindowSystem.h"

//////////////////////////////////////////////////////////////////////////////
#define MAX_SLOTS 16

enum evdev_event_type {
    EVDEV_ABSOLUTE_MOTION    = (1 << 0),
    EVDEV_ABSOLUTE_MT_DOWN   = (1 << 1),
    EVDEV_ABSOLUTE_MT_MOTION = (1 << 2),
    EVDEV_ABSOLUTE_MT_UP     = (1 << 3),
    EVDEV_RELATIVE_MOTION    = (1 << 4)
};

enum evdev_device_capability {
    EVDEV_KEYBOARD   = (1 << 0),
    EVDEV_BUTTON     = (1 << 1),
    EVDEV_MOTION_ABS = (1 << 2),
    EVDEV_MOTION_REL = (1 << 3),
    EVDEV_TOUCH      = (1 << 4)
};

enum key_state_update {
    STATE_UPDATE_AUTOMATIC,
    STATE_UPDATE_NONE
};

class WaylandEvdevInputEvent;
struct evdev_dispatch;

struct evdev_input_device {
    WaylandEvdevInputEvent *master;
    struct wl_list          link;
    struct wl_event_source *source;
    struct evdev_dispatch  *dispatch;
    char *devnode;
    int   fd;
    struct {
        int min_x, max_x, min_y, max_y;
        int32_t x, y;
    } abs;

    struct {
        int slot;
        int32_t x[MAX_SLOTS];
        int32_t y[MAX_SLOTS];
    } mt;
    struct mtdev* mtdev;

    struct {
        wl_fixed_t dx, dy;
    } rel;

    unsigned int pending_events;
    unsigned int caps;
    int isMt;
};

struct evdev_dispatch_interface {
    // Process an evdev input event
    void (*process)(struct evdev_dispatch *dispatch,
                    struct evdev_input_device *device,
                    struct input_event *event,
                    uint32_t time);

    // Destroy an event dispatch handler and free all its resources
    void (*destroy)(struct evdev_dispatch *dispatch);
};

struct evdev_dispatch {
    struct evdev_dispatch_interface *interface;
};

//////////////////////////////////////////////////////////////////////////////

// Default values
#define DEFAULT_CONSTANT_ACCEL_NUMERATOR 50
#define DEFAULT_MIN_ACCEL_FACTOR 0.16
#define DEFAULT_MAX_ACCEL_FACTOR 1.0
#define DEFAULT_HYSTERESIS_MARGIN_DENOMINATOR 700.0

enum touchpad_model {
    TOUCHPAD_MODEL_UNKNOWN = 0,
    TOUCHPAD_MODEL_SYNAPTICS,
    TOUCHPAD_MODEL_ALPS,
    TOUCHPAD_MODEL_APPLETOUCH,
    TOUCHPAD_MODEL_ELANTECH
};

#define TOUCHPAD_EVENT_NONE           0
#define TOUCHPAD_EVENT_ABSOLUTE_ANY  (1 << 0)
#define TOUCHPAD_EVENT_ABSOLUTE_X    (1 << 1)
#define TOUCHPAD_EVENT_ABSOLUTE_Y    (1 << 2)
#define TOUCHPAD_EVENT_REPORT        (1 << 3)

struct touchpad_model_spec {
    short vendor;
    short product;
    enum touchpad_model model;
};

enum touchpad_state {
    TOUCHPAD_STATE_NONE = 0,
    TOUCHPAD_STATE_TOUCH,
    TOUCHPAD_STATE_PRESS
};

#define TOUCHPAD_HISTORY_LENGTH 4

struct touchpad_motion {
    int32_t x;
    int32_t y;
};

enum touchpad_fingers_state {
    TOUCHPAD_FINGERS_ONE   = (1 << 0),
    TOUCHPAD_FINGERS_TWO   = (1 << 1),
    TOUCHPAD_FINGERS_THREE = (1 << 2)
};

struct touchpad_dispatch {
    struct evdev_dispatch base;
    struct evdev_input_device *device;

    enum touchpad_model model;
    enum touchpad_state state;
    int finger_state;
    int last_finger_state;

    double constant_accel_factor;
    double min_accel_factor;
    double max_accel_factor;

    unsigned int event_mask;
    unsigned int event_mask_filter;

    int reset;

    struct {
        int32_t x;
        int32_t y;
    } hw_abs;

    int has_pressure;
    struct {
        int32_t touch_low;
        int32_t touch_high;
        int32_t press;
    } pressure;

    struct {
        int32_t margin_x;
        int32_t margin_y;
        int32_t center_x;
        int32_t center_y;
    } hysteresis;

    struct touchpad_motion motion_history[TOUCHPAD_HISTORY_LENGTH];
    int motion_index;
    unsigned int motion_count;

    struct wl_list motion_filters;
};

struct motion_params {
    double dx, dy;
};

struct motion_filter;

struct motion_filter_interface {
    void (*filter)(struct motion_filter *filter,
                   struct motion_params *motion,
                   void *data, uint32_t time);
    void (*destroy)(struct motion_filter *filter);
};

struct motion_filter {
    struct motion_filter_interface *interface;
    struct wl_list link;
};

typedef double (*accel_profile_func_t)(struct motion_filter *filter,
                                       void *data,
                                       double velocity,
                                       uint32_t time);

struct pointer_tracker {
    double   dx;
    double   dy;
    uint32_t time;
    int      dir;
};

struct pointer_accelerator {
    struct motion_filter base;
    accel_profile_func_t profile;

    double velocity;
    double last_velocity;
    int    last_dx;
    int    last_dy;

    struct pointer_tracker *trackers;
    int cur_tracker;
};

enum directions {
    N  = 1 << 0,
    NE = 1 << 1,
    E  = 1 << 2,
    SE = 1 << 3,
    S  = 1 << 4,
    SW = 1 << 5,
    W  = 1 << 6,
    NW = 1 << 7,
    UNDEFINED_DIRECTION = 0xff
};

//////////////////////////////////////////////////////////////////////////////

class WaylandEvdevInputEvent : public WaylandInputEvent
{
// Properties
private:
    struct udev*   m_udev;
    struct wl_list m_deviceList;
    int m_screenWidth;
    int m_screenHeight;

// Methods
public:
    WaylandEvdevInputEvent(WaylandBaseWindowSystem *windowSystem);
    virtual ~WaylandEvdevInputEvent();

    virtual void setupInputEvent();

    // Default event handler
    static void fallbackProcess(struct evdev_dispatch *dispatch,
                                struct evdev_input_device *device,
                                struct input_event *e,
                                uint32_t time);
    static void fallbackDestroy(struct evdev_dispatch *dispatch);

    // Multi-touch event handler
    static void touchpadProcess(struct evdev_dispatch *dispatch,
                                struct evdev_input_device *device,
                                struct input_event *e,
                                uint32_t time);
    static void touchpadDestroy(struct evdev_dispatch *dispatch);

    static void flushMotion(struct evdev_input_device *device, uint32_t time);

private:
    static int  handleInputEvent(int fd, uint32_t mask, void *data);
    static void processEvents(struct evdev_input_device *device,
                              struct input_event *ev, int count);

    // Default event handler
    static void evdevProcessRelative(struct evdev_input_device *device,
                                     uint32_t time, struct input_event *e);
    static void evdevProcessAbsolute(struct evdev_input_device *device,
                                     struct input_event *e);
    static void evdevProcessKey(struct evdev_input_device *device,
                                uint32_t time, struct input_event *e);

    // Multi-touch event handler
    static void touchpadProcessAbsolute(struct touchpad_dispatch *touchpad,
                                        struct evdev_input_device *device,
                                        struct input_event *e);
    static void touchpadProcessKey(struct touchpad_dispatch *touchpad,
                                   struct evdev_input_device *device,
                                   struct input_event *e,
                                   uint32_t time);
    static void touchpadUpdateState(struct touchpad_dispatch *touchpad,
                                    uint32_t time);

    // Notifier
    static void notifyButton(struct evdev_input_device *device, uint32_t time,
                             int32_t button, enum wl_pointer_button_state state);
    static void notifyMotion(struct evdev_input_device *device, uint32_t time,
                             wl_fixed_t fx, wl_fixed_t fy);
    static void notifyKey(struct evdev_input_device *device, uint32_t time,
                          uint32_t key, enum wl_keyboard_key_state state,
                          bool bUpdateAutomatic);
    static void notifyTouch(struct evdev_input_device *device);

    bool addDevices();
    void addDevice(struct udev_device *udevDevice);
    void removeDevice(struct evdev_input_device *device);
    void createInputDevice(struct wl_display *display, const char *path);
    int  configureDevice(struct evdev_input_device *device);
    void dispatchProcess(struct evdev_input_device *device,
                         struct input_event *ev,
                         uint32_t time);
    void notifyKeyboardFocus();
    void notifyKeyboardFocusIn(struct wl_array *keys, enum key_state_update updateState);
    void updateModifierState(struct wl_seat *wlSeat, uint32_t serial, uint32_t key,
                             enum wl_keyboard_key_state state);
    void notifyModifiers(struct wl_seat *wlSeat, uint32_t serial);

    struct evdev_dispatch* createTouchpad(struct evdev_input_device *device);
    void configureTouchpad(struct touchpad_dispatch *touchpad,
                           struct evdev_input_device *device);

};

#endif /* _WAYLANDEVDEVINPUTEVENT_H_ */
