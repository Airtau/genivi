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
#include <fcntl.h>
#include <unistd.h>
#include <libudev.h>
#include <mtdev.h>
#include <math.h>
#include "InputManager.h"
#include "WindowSystems/WaylandEvdevInputEvent.h"

#define ARRAY_LENGTH(a) (sizeof (a) / sizeof (a)[0])

#define WL_UNUSED(A) (A)=(A)
static const char default_seat[] = "seat0";

// copied from udev/extras/input_id/input_id.c
// we must use this kernel-compatible implementation
#define BITS_PER_LONG (sizeof(unsigned long) * 8)
#define NBITS(x) ((((x)-1)/BITS_PER_LONG)+1)
#define OFF(x)  ((x)%BITS_PER_LONG)
#define BIT(x)  (1UL<<OFF(x))
#define LONG(x) ((x)/BITS_PER_LONG)
#define TEST_BIT(array, bit)    ((array[LONG(bit)] >> OFF(bit)) & 1)
// end copied

#define MAX_VELOCITY_DIFF    1.0
#define MOTION_TIMEOUT       300 // (ms)
#define NUM_POINTER_TRACKERS 16

struct evdev_dispatch_interface touchpad_interface = {
    WaylandEvdevInputEvent::touchpadProcess,
    WaylandEvdevInputEvent::touchpadDestroy,
};

struct evdev_dispatch_interface fallback_interface = {
    WaylandEvdevInputEvent::fallbackProcess,
    WaylandEvdevInputEvent::fallbackDestroy,
};

// Function prototypes
void acceleratorFilter (struct motion_filter *, struct motion_params *, void *, uint32_t);
void acceleratorDestroy(struct motion_filter *);

struct motion_filter_interface accelerator_interface = {
    acceleratorFilter,
    acceleratorDestroy,
};

static struct touchpad_model_spec touchpad_spec_table[] = {
    {0x0002, 0x0007, TOUCHPAD_MODEL_SYNAPTICS},
    {0x0002, 0x0008, TOUCHPAD_MODEL_ALPS},
    {0x05ac, 0x0000, TOUCHPAD_MODEL_APPLETOUCH},
    {0x0002, 0x000e, TOUCHPAD_MODEL_ELANTECH},
    {0x0000, 0x0000, TOUCHPAD_MODEL_UNKNOWN}
};

////////////////////////////////////////////////////////////////////////////

static int
isMotionEvent(struct input_event *e)
{
    switch (e->type){
    case EV_REL:
        switch (e->code){
        case REL_X:
        case REL_Y:
            return 1;
        }
        break;
    case EV_ABS:
        switch (e->code){
        case ABS_X:
        case ABS_Y:
        case ABS_MT_POSITION_X:
        case ABS_MT_POSITION_Y:
            return 1;
        }
        break;
    }
    return 0;
}

static enum touchpad_model
getTouchpadModel(struct evdev_input_device *device)
{
    struct input_id id;
    unsigned int i;

    if (ioctl(device->fd, EVIOCGID, &id) < 0)
        return TOUCHPAD_MODEL_UNKNOWN;

    for (i = 0; i < sizeof(touchpad_spec_table); ++i){
        if (touchpad_spec_table[i].vendor == id.vendor &&
            (!touchpad_spec_table[i].product ||
            touchpad_spec_table[i].product == id.product)){
            return touchpad_spec_table[i].model;
        }
    }

    return TOUCHPAD_MODEL_UNKNOWN;
}

static void
configureTouchpadPressure(struct touchpad_dispatch *touchpad,
                          int32_t pressure_min, int32_t pressure_max)
{
    int32_t range = pressure_max - pressure_min + 1;
    touchpad->has_pressure = 1;

    // Magic numbers from xf86-input-synaptics
    switch (touchpad->model){
    case TOUCHPAD_MODEL_ELANTECH:
        touchpad->pressure.touch_low  = pressure_min + 1;
        touchpad->pressure.touch_high = pressure_min + 1;
        break;
    default:
        touchpad->pressure.touch_low  = pressure_min + range * (25.0/256.0);
        touchpad->pressure.touch_high = pressure_min + range * (30.0/256.0);
        break;
    }

    touchpad->pressure.press = pressure_min + range;
}

static double
touchpadProfile(struct motion_filter *filter, void *data,
                double velocity, uint32_t time)
{
    WL_UNUSED(filter);
    WL_UNUSED(time);
    struct touchpad_dispatch *touchpad = (struct touchpad_dispatch*)data;
    double accel_factor;

    accel_factor = velocity * touchpad->constant_accel_factor;

    if (accel_factor > touchpad->max_accel_factor)
        accel_factor = touchpad->max_accel_factor;
    else if (accel_factor < touchpad->min_accel_factor)
        accel_factor = touchpad->min_accel_factor;

    return accel_factor;
}

static motion_filter*
createPointerAccelatorFilter(accel_profile_func_t profile)
{
    struct pointer_accelerator *filter;

    filter = (struct pointer_accelerator*)malloc(sizeof(*filter));
    if (filter == NULL)
        return NULL;

    filter->base.interface = &accelerator_interface;
    wl_list_init(&filter->base.link);

    filter->profile = profile;
    filter->last_velocity = 0.0;
    filter->last_dx = 0;
    filter->last_dy = 0;

    filter->trackers = (struct pointer_tracker*)
        calloc(NUM_POINTER_TRACKERS, sizeof(*filter->trackers));
    filter->cur_tracker = 0;

    return &filter->base;
}

static evdev_dispatch*
createFallbackDispatch()
{
    struct evdev_dispatch *dispatch =
        (struct evdev_dispatch*)malloc(sizeof(*dispatch));
    if (dispatch ==   NULL)
        return NULL;

    dispatch->interface = &fallback_interface;

    return dispatch;
}

////////////////////////////////////////////////////////////////////////

static void
processTouch(struct evdev_input_device *device, struct input_event *e,
             int screen_width, int screen_height)
{
    switch (e->code) {
    case ABS_MT_SLOT:
        device->mt.slot = e->value;
        break;
    case ABS_MT_TRACKING_ID:
        if (e->value >= 0)
            device->pending_events |= EVDEV_ABSOLUTE_MT_DOWN;
        else
            device->pending_events |= EVDEV_ABSOLUTE_MT_UP;
        break;
    case ABS_MT_POSITION_X:
        device->mt.x[device->mt.slot] =
            (e->value - device->abs.min_x) * screen_width /
            (device->abs.max_x - device->abs.min_x) +
            0; //device->output->x;
        device->pending_events |= EVDEV_ABSOLUTE_MT_MOTION;
        break;
    case ABS_MT_POSITION_Y:
        device->mt.y[device->mt.slot] =
            (e->value - device->abs.min_y) * screen_height /
            (device->abs.max_y - device->abs.min_y) +
            0; //device->output->y;
        device->pending_events |= EVDEV_ABSOLUTE_MT_MOTION;
        break;
    }
}

static void
processAbsoluteMotion(struct evdev_input_device *device, struct input_event *e,
                      int screen_width, int screen_height)
{
    switch (e->code) {
    case ABS_X:
        device->abs.x =
            (e->value - device->abs.min_x) * screen_width /
            (device->abs.max_x - device->abs.min_x) +
            0; //device->output->x;
        device->pending_events |= EVDEV_ABSOLUTE_MOTION;
        break;
    case ABS_Y:
        device->abs.y =
            (e->value - device->abs.min_y) * screen_height /
            (device->abs.max_y - device->abs.min_y) +
            0; //device->output->y;
        device->abs.x = screen_width - device->abs.x;
        device->pending_events |= EVDEV_ABSOLUTE_MOTION;
        break;
    // The following is the effective code only from a specific model
    /****
    case ABS_X:
        device->abs.y = (e->value - device->abs.min_x) * screen_height /
                        (device->abs.max_x - device->abs.min_x);
        device->pending_events |= EVDEV_ABSOLUTE_MOTION;
        break;
    case ABS_Y:
        device->abs.x = (e->value - device->abs.min_y) * screen_width /
                        (device->abs.max_y - device->abs.min_y);
        device->abs.x = screen_width - device->abs.x;
        device->pending_events |= EVDEV_ABSOLUTE_MOTION;
        break;
     ****/
    }
}

////////////////////////////////////////////////////////////////////////

static int
hysteresis(int in, int center, int margin)
{
    int diff = in - center;
    if (abs(diff) <= margin)
        return center;

    if (diff > margin)
        return center + diff - margin;
    else if (diff < -margin)
        return center + diff + margin;
    return center + diff;
}

static inline struct touchpad_motion*
motionHistoryOffset(struct touchpad_dispatch *touchpad, int offset)
{
    int offsetIndex =
        (touchpad->motion_index - offset + TOUCHPAD_HISTORY_LENGTH) %
        TOUCHPAD_HISTORY_LENGTH;

    return &touchpad->motion_history[offsetIndex];
}

static double
estimateDelta(int x0, int x1, int x2, int x3)
{
    return (x0 + x1 - x2 - x3) / 4;
}

static void
touchpadGetDelta(struct touchpad_dispatch *touchpad, double *dx, double *dy)
{
    *dx = estimateDelta(motionHistoryOffset(touchpad, 0)->x,
                        motionHistoryOffset(touchpad, 1)->x,
                        motionHistoryOffset(touchpad, 2)->x,
                        motionHistoryOffset(touchpad, 3)->x);
    *dy = estimateDelta(motionHistoryOffset(touchpad, 0)->y,
                        motionHistoryOffset(touchpad, 1)->y,
                        motionHistoryOffset(touchpad, 2)->y,
                        motionHistoryOffset(touchpad, 3)->y);
}

static void
filterDispatch(struct motion_filter *filter, struct motion_params *motion,
               void *data, uint32_t time)
{
    filter->interface->filter(filter, motion, data, time);
}

static void
filterMotion(struct touchpad_dispatch *touchpad,
             double *dx, double *dy, uint32_t time)
{
    struct motion_filter *filter;
    struct motion_params  motion;

    motion.dx = *dx;
    motion.dy = *dy;

    wl_list_for_each(filter, &touchpad->motion_filters, link){
        filterDispatch(filter, &motion, touchpad, time);
    }

    *dx = motion.dx;
    *dy = motion.dy;
}

static int
getDirection(int dx, int dy)
{
    int dir = UNDEFINED_DIRECTION;
    int d1, d2;
    double r;

    if (abs(dx) < 2 && abs(dy) < 2) {
        if (dx > 0 && dy > 0)
            dir = S | SE | E;
        else if (dx > 0 && dy < 0)
            dir = N | NE | E;
        else if (dx < 0 && dy > 0)
            dir = S | SW | W;
        else if (dx < 0 && dy < 0)
            dir = N | NW | W;
        else if (dx > 0)
            dir = NW | W | SW;
        else if (dx < 0)
            dir = NE | E | SE;
        else if (dy > 0)
            dir = SE | S | SW;
        else if (dy < 0)
            dir = NE | N | NW;
    }
    else {
        // Calculate r within the interval  [0 to 8)
        //
        // r = [0 .. 2π] where 0 is North
        // d_f = r / 2π  ([0 .. 1))
        // d_8 = 8 * d_f
        //
        r = atan2(dy, dx);
        r = fmod(r + 2.5*M_PI, 2*M_PI);
        r *= 4*M_1_PI;

        // Mark one or two close enough octants
        d1 = (int)(r + 0.9) % 8;
        d2 = (int)(r + 0.1) % 8;

        dir = (1 << d1) | (1 << d2);
    }

    return dir;
}

static void
feedTrackers(struct pointer_accelerator *accel,
             double dx, double dy, uint32_t time)
{
    int i, current;
    struct pointer_tracker *trackers = accel->trackers;

    for (i = 0; i < NUM_POINTER_TRACKERS; ++i){
        trackers[i].dx += dx;
        trackers[i].dy += dy;
    }

    current = (accel->cur_tracker + 1) % NUM_POINTER_TRACKERS;
    accel->cur_tracker = current;

    trackers[current].dx = 0.0;
    trackers[current].dy = 0.0;
    trackers[current].time = time;
    trackers[current].dir  = getDirection(dx, dy);
}

static struct pointer_tracker*
trackerByOffset(struct pointer_accelerator *accel, unsigned int offset)
{
    unsigned int index =
        (accel->cur_tracker + NUM_POINTER_TRACKERS - offset)
        % NUM_POINTER_TRACKERS;
    return &accel->trackers[index];
}

static double
calculateTrackerVelocity(struct pointer_tracker *tracker, uint32_t time)
{
    int dx;
    int dy;
    double distance;

    dx = tracker->dx;
    dy = tracker->dy;
    distance = sqrt(dx*dx + dy*dy);
    return distance / (double)(time - tracker->time);
}

static double
calculateVelocity(struct pointer_accelerator *accel, uint32_t time)
{
    struct pointer_tracker *tracker;
    double velocity;
    double result = 0.0;
    double initial_velocity;
    double velocity_diff;
    unsigned int offset;

    unsigned int dir = trackerByOffset(accel, 0)->dir;

    // Find first velocity
    for (offset = 1; offset < NUM_POINTER_TRACKERS; offset++) {
        tracker = trackerByOffset(accel, offset);

        if (time <= tracker->time)
            continue;

        result = initial_velocity =
            calculateTrackerVelocity(tracker, time);
        if (initial_velocity > 0.0)
            break;
    }

    // Find least recent vector within a timelimit, maximum velocity diff
    // and direction threshold.
    for (; offset < NUM_POINTER_TRACKERS; offset++) {
        tracker = trackerByOffset(accel, offset);

        // Stop if too far away in time
        if (time - tracker->time > MOTION_TIMEOUT ||
            tracker->time > time)
            break;

        // Stop if direction changed
        dir &= tracker->dir;
        if (dir == 0)
            break;

        velocity = calculateTrackerVelocity(tracker, time);

        // Stop if velocity differs too much from initial
        velocity_diff = fabs(initial_velocity - velocity);
        if (velocity_diff > MAX_VELOCITY_DIFF)
            break;

        result = velocity;
    }

    return result;
}

static double
accelerationProfile(struct pointer_accelerator *accel,
                    void *data, double velocity, uint32_t time)
{
    return accel->profile(&accel->base, data, velocity, time);
}

static double
calculateAcceleration(struct pointer_accelerator *accel,
                      void *data, double velocity, uint32_t time)
{
    double factor;

    factor  = accelerationProfile(accel, data, velocity, time);
    factor += accelerationProfile(accel, data, accel->last_velocity, time);
    factor += 4.0 *
              accelerationProfile(accel, data,
                                 (accel->last_velocity + velocity) / 2,
                                  time);
    factor = factor / 6.0;
    return factor;
}

static double
softenDelta(double lastDelta, double delta)
{
    if (delta < -1.0 || delta > 1.0){
        if (delta > lastDelta)
            return delta - 0.5;
        else if (delta < lastDelta)
            return delta + 0.5;
    }
    return delta;
}

static void
applySoftening(struct pointer_accelerator *accel,
               struct motion_params *motion)
{
    motion->dx = softenDelta(accel->last_dx, motion->dx);
    motion->dy = softenDelta(accel->last_dy, motion->dy);
}

void
acceleratorFilter(struct motion_filter *filter, struct motion_params *motion,
                  void *data, uint32_t time)
{
    struct pointer_accelerator *accel = (struct pointer_accelerator*)filter;
    double velocity;
    double accel_value;

    feedTrackers(accel, motion->dx, motion->dy, time);
    velocity = calculateVelocity(accel, time);
    accel_value = calculateAcceleration(accel, data, velocity, time);

    motion->dx = accel_value * motion->dx;
    motion->dy = accel_value * motion->dy;

    applySoftening(accel, motion);

    accel->last_dx = motion->dx;
    accel->last_dy = motion->dy;

    accel->last_velocity = velocity;
}

void
acceleratorDestroy(struct motion_filter *filter)
{
    struct pointer_accelerator *accel = (struct pointer_accelerator*)filter;

    free(accel->trackers);
    free(accel);
}

/// WaylandEvdevInputEvent /////////////////////////////////////////////////

WaylandEvdevInputEvent::WaylandEvdevInputEvent(WaylandBaseWindowSystem* windowSystem)
: WaylandInputEvent(windowSystem)
, m_udev(NULL)
, m_screenWidth(0)
, m_screenHeight(0)
{
    wl_list_init(&m_deviceList);
}

WaylandEvdevInputEvent::~WaylandEvdevInputEvent()
{
    if (m_udev){
        udev_unref(m_udev);
    }

    struct evdev_input_device *device, *next;

    wl_list_for_each_safe(device, next, &m_deviceList, link){
        removeDevice(device);
    }
}

void
WaylandEvdevInputEvent::removeDevice(struct evdev_input_device *device)
{
    struct evdev_dispatch *dispatch = device->dispatch;

    if (dispatch)
        dispatch->interface->destroy(dispatch);

    wl_event_source_remove(device->source);
    wl_list_remove(&device->link);
    if (device->mtdev)
        mtdev_close_delete(device->mtdev);
    close(device->fd);
    free(device->devnode);
    free(device);
}

void
WaylandEvdevInputEvent::setupInputEvent()
{
    LOG_INFO("WaylandEvdevInputEvent", "setupInputEvent IN");

    WaylandInputEvent::setupInputEvent();

    m_screenWidth  = m_windowSystem->getWindowWidth();
    m_screenHeight = m_windowSystem->getWindowHeight();

    do {
        bool bRet = addDevices();
        if (!bRet){
            break;
        }
    } while (0);

    LOG_INFO("WaylandEvdevInputEvent", "setupInputEvent OUT");
}

bool
WaylandEvdevInputEvent::addDevices()
{
    if (!m_udev)
        m_udev = udev_new();
    if (!m_udev){
        LOG_ERROR("WaylandEvdevInputEvent", "Failed to initialize udev context");
        return false;
    }

    struct udev_enumerate *e = udev_enumerate_new(m_udev);
    udev_enumerate_add_match_subsystem(e, "input");
    udev_enumerate_scan_devices(e);

    struct udev_list_entry *entry;
    const char *path, *sysname;
    struct udev_device *device;
    udev_list_entry_foreach(entry, udev_enumerate_get_list_entry(e)){
        path = udev_list_entry_get_name(entry);
        device = udev_device_new_from_syspath(m_udev, path);

        sysname = udev_device_get_sysname(device);
        if (strncmp("event", sysname, 5) != 0){
            udev_device_unref(device);
            continue;
        }

        addDevice(device);
        udev_device_unref(device);
    }
    udev_enumerate_unref(e);

    notifyKeyboardFocus();

    if (wl_list_empty(&m_deviceList)){
        LOG_WARNING("WaylandEvdevInputEvent", "No input devices on entering service");
    }

    return true;
}

void
WaylandEvdevInputEvent::addDevice(struct udev_device *udevDevice)
{
    const char *devnode;
    const char *device_seat;

    device_seat = udev_device_get_property_value(udevDevice, "ID_SEAT");
    if (!device_seat)
        device_seat = default_seat;

    devnode = udev_device_get_devnode(udevDevice);
    createInputDevice(m_windowSystem->getNativeDisplayHandle(), devnode);
}

void
WaylandEvdevInputEvent::createInputDevice(struct wl_display *display, const char *path)
{
    struct evdev_input_device *device;
    struct wl_event_loop      *eventLoop;

    device = (struct evdev_input_device*)malloc(sizeof(*device));
    if (device == NULL){
        return;
    }

    device->master   = this;
    device->isMt     = 0;
    device->mtdev    = NULL;
    device->devnode  = strdup(path);
    device->mt.slot  = -1;
    device->rel.dx   = 0;
    device->rel.dy   = 0;
    device->dispatch = NULL;

    device->fd = open(path, O_RDWR | O_NONBLOCK);
    if (device->fd < 0){
        goto err0;
    }

    if (configureDevice(device) < 0){
        goto err1;
    }

    // If the dispatch was not set up use the fallback
    if (device->dispatch == NULL)
        device->dispatch = createFallbackDispatch();
    if (device->dispatch == NULL)
        goto err1;

    if (device->isMt){
        device->mtdev = mtdev_new_open(device->fd);
        if  (!device->mtdev){
            LOG_WARNING("WaylandEvdevInputEvent", "mtdev failed to open for " << path);
        }
    }

    eventLoop = wl_display_get_event_loop(display);
    device->source = wl_event_loop_add_fd(eventLoop, device->fd, WL_EVENT_READABLE,
                                          WaylandEvdevInputEvent::handleInputEvent,
                                          device);
    if (device->source == NULL){
        goto err2;
    }

    wl_list_insert(m_deviceList.prev, &(device->link));

    return;

err2:
    device->dispatch->interface->destroy(device->dispatch);
err1:
    close(device->fd);
err0:
    free(device->devnode);
    free(device);
    return;
}

int
WaylandEvdevInputEvent::configureDevice(struct evdev_input_device *device)
{
    struct input_absinfo absinfo;
    unsigned long ev_bits[NBITS(EV_MAX)];
    unsigned long abs_bits[NBITS(ABS_MAX)];
    unsigned long rel_bits[NBITS(ABS_MAX)];
    unsigned long key_bits[NBITS(KEY_MAX)];

    int hasKey = 0;
    int hasAbs = 0;
    int i;
    device->caps = 0;

    ioctl(device->fd, EVIOCGBIT(0, sizeof(ev_bits)), ev_bits);
    if (TEST_BIT(ev_bits, EV_ABS)){
        hasAbs = 1;

        ioctl(device->fd, EVIOCGBIT(EV_ABS, sizeof(abs_bits)), abs_bits);
        if (TEST_BIT(abs_bits, ABS_X)){
            ioctl(device->fd, EVIOCGABS(ABS_X), &absinfo);
            device->abs.min_x = absinfo.minimum;
            device->abs.max_x = absinfo.maximum;
            device->caps |= EVDEV_MOTION_ABS;
        }
        if (TEST_BIT(abs_bits, ABS_Y)){
            ioctl(device->fd, EVIOCGABS(ABS_Y), &absinfo);
            device->abs.min_y = absinfo.minimum;
            device->abs.max_y = absinfo.maximum;
            device->caps |= EVDEV_MOTION_ABS;
        }
        if (TEST_BIT(abs_bits, ABS_MT_SLOT)){
            device->isMt = 1;
            device->mt.slot = 0;
            device->caps |= EVDEV_TOUCH;
        }
    }
    if (TEST_BIT(ev_bits, EV_REL)){
        ioctl(device->fd, EVIOCGBIT(EV_REL, sizeof(rel_bits)), rel_bits);
        if (TEST_BIT(rel_bits, REL_X) || TEST_BIT(rel_bits, REL_Y)){
            device->caps |= EVDEV_MOTION_REL;
        }
    }
    if (TEST_BIT(ev_bits, EV_KEY)){
        hasKey = 1;
        ioctl(device->fd, EVIOCGBIT(EV_KEY, sizeof(key_bits)), key_bits);
        if (TEST_BIT(key_bits, BTN_TOOL_FINGER) &&
            !TEST_BIT(key_bits, BTN_TOOL_PEN) && hasAbs){
            device->dispatch = createTouchpad(device);
        }

        for (i = KEY_ESC; i < KEY_MAX; ++i){
            if (i >= BTN_MISC && i < KEY_OK)
                continue;
            if (TEST_BIT(key_bits, i)){
                device->caps |= EVDEV_KEYBOARD;
                break;
            }
        }
        for (i = BTN_MISC; i < KEY_OK; ++i){
            if (TEST_BIT(key_bits, i)){
                device->caps |= EVDEV_BUTTON;
                break;
            }
        }
    }
    if (TEST_BIT(ev_bits, EV_LED)){
        device->caps |= EVDEV_KEYBOARD;
    }

    // This rule tries to catch accelerometer devices and opt out. We may
    // want to adjust the protocol later adding a proper event for dealing
    // with accelerometers and implement here accordingly
    if (hasAbs && !hasKey && !device->isMt)
        return -1;

#if 0
    fprintf(stdout, "DEVICE: [%s] information\n", device->devnode);
    fprintf(stdout, "        capabilities: EVDEV_KEYBOARD   %s\n"
                    "                      EVDEV_BUTTON     %s\n"
                    "                      EVDEV_MOTION_ABS %s\n"
                    "                      EVDEV_MOTION_REL %s\n"
                    "                      EVDEV_TOUCH      %s\n",
            (device->caps & EVDEV_KEYBOARD)   ? "TRUE" : "FALSE",
            (device->caps & EVDEV_BUTTON)     ? "TRUE" : "FALSE",
            (device->caps & EVDEV_MOTION_ABS) ? "TRUE" : "FALSE",
            (device->caps & EVDEV_MOTION_REL) ? "TRUE" : "FALSE",
            (device->caps & EVDEV_TOUCH)      ? "TRUE" : "FALSE");
    if (device->caps & EVDEV_MOTION_ABS){
        fprintf(stdout, "        abs: min_x(%4d), min_y(%4d)\n", device->abs.min_x, device->abs.min_y);
        fprintf(stdout, "             max_x(%4d), max_y(%4d)\n", device->abs.max_x, device->abs.max_y);
        fprintf(stdout, "                 x(%4d),     y(%4d)\n", device->abs.x, device->abs.y);
    }
    fprintf(stdout, "\n");
#endif

    if ((device->caps & (EVDEV_MOTION_ABS | EVDEV_MOTION_REL | EVDEV_BUTTON))){
        initPointerDevice();
    }
    if ((device->caps & (EVDEV_KEYBOARD))){
        initKeyboardDevice(NULL);
    }
    if ((device->caps & (EVDEV_TOUCH))){
        initTouchDevice();
    }

    return 0;
}

void
WaylandEvdevInputEvent::notifyKeyboardFocus()
{
    struct evdev_input_device *device;
    struct wl_array keys;
    char evdev_keys[(KEY_CNT + 7) / 8], all_keys[(KEY_CNT + 7) / 8];
    uint32_t *k;
    unsigned int i, set;
    int ret;

    memset(all_keys, 0, sizeof(all_keys));
    wl_list_for_each(device, &m_deviceList, link){
        memset(evdev_keys, 0, sizeof(evdev_keys));
        ret = ioctl(device->fd, EVIOCGKEY(sizeof(evdev_keys)), evdev_keys);
        if (ret < 0){
            LOG_WARNING("WaylandEvdevInputEvent", "Failed to get keys for device: " <<
                        device->devnode);
            continue;
        }
        for (i = 0; i < ARRAY_LENGTH(evdev_keys); ++i){
            all_keys[i] |= evdev_keys[i];
        }
    }

    wl_array_init(&keys);
    for (i = 0; i < KEY_CNT; ++i){
        set = all_keys[i >> 3] & (1 << (i & 7));
        if (set){
            k = (uint32_t*)wl_array_add(&keys, sizeof(*k));
            *k = i;
        }
    }

    notifyKeyboardFocusIn(&keys, STATE_UPDATE_AUTOMATIC);

    wl_array_release(&keys);
}

void
WaylandEvdevInputEvent::notifyKeyboardFocusIn(struct wl_array *keys,
                                              enum key_state_update updateState)
{
    struct wl_seat *wlSeat;
    uint32_t *k, serial;

    if ((wlSeat = m_inputDevice->seat()) == NULL){
        return;
    }
    if (!wlSeat->keyboard){
        return;
    }
    serial = wl_display_next_serial(m_inputDevice->display());
    wl_array_init(&wlSeat->keyboard->keys);
    wl_array_copy(&wlSeat->keyboard->keys, keys);

    struct wl_array *array = &wlSeat->keyboard->keys;
    for (k = (uint32_t*)array->data;
        (const char*)k < (const char*)array->data + array->size;
        ++k){
        if (updateState == STATE_UPDATE_AUTOMATIC){
            updateModifierState(wlSeat, serial, *k, WL_KEYBOARD_KEY_STATE_PRESSED);
        }
    }
}

void
WaylandEvdevInputEvent::updateModifierState(struct wl_seat *wlSeat, uint32_t serial,
                                            uint32_t key, enum wl_keyboard_key_state state)
{
    enum xkb_key_direction direction;

    if (state == WL_KEYBOARD_KEY_STATE_PRESSED)
        direction = XKB_KEY_DOWN;
    else
        direction = XKB_KEY_UP;

    // Offset the keycode by 8, as the evdev XKB rules reflect X's
    // broken keycode system, which starts at 8.
    xkb_state_update_key(m_xkbState.state, key + 8, direction);

    notifyModifiers(wlSeat, serial);
}

struct evdev_dispatch*
WaylandEvdevInputEvent::createTouchpad(struct evdev_input_device *device)
{
    struct touchpad_dispatch *touchpad;

    touchpad = (struct touchpad_dispatch*)malloc(sizeof(*touchpad));
    if (touchpad == NULL)
        return NULL;

    touchpad->base.interface = &touchpad_interface;

    touchpad->device = device;
    wl_list_init(&touchpad->motion_filters);

    configureTouchpad(touchpad, device);

    return &touchpad->base;
}

void
WaylandEvdevInputEvent::configureTouchpad(struct touchpad_dispatch *touchpad,
                                          struct evdev_input_device *device)
{
    struct motion_filter *accel;

    struct input_absinfo absinfo;
    unsigned long abs_bits[NBITS(ABS_MAX)];

    double width;
    double height;
    double diagonal;

    // Detect model
    touchpad->model = getTouchpadModel(device);

    // Configure pressure
    ioctl(device->fd, EVIOCGBIT(EV_ABS, sizeof(abs_bits)), abs_bits);
    if (TEST_BIT(abs_bits, ABS_PRESSURE)) {
        ioctl(device->fd, EVIOCGABS(ABS_PRESSURE), &absinfo);
        configureTouchpadPressure(touchpad,
                                  absinfo.minimum,
                                  absinfo.maximum);
    }

    // Configure acceleration factor
    width = abs(device->abs.max_x - device->abs.min_x);
    height = abs(device->abs.max_y - device->abs.min_y);
    diagonal = sqrt(width*width + height*height);

    touchpad->constant_accel_factor =
        DEFAULT_CONSTANT_ACCEL_NUMERATOR / diagonal;

    touchpad->min_accel_factor = DEFAULT_MIN_ACCEL_FACTOR;
    touchpad->max_accel_factor = DEFAULT_MAX_ACCEL_FACTOR;

    touchpad->hysteresis.margin_x = diagonal / DEFAULT_HYSTERESIS_MARGIN_DENOMINATOR;
    touchpad->hysteresis.margin_y = diagonal / DEFAULT_HYSTERESIS_MARGIN_DENOMINATOR;
    touchpad->hysteresis.center_x = 0;
    touchpad->hysteresis.center_y = 0;

    // Configure acceleration profile
    accel = createPointerAccelatorFilter(touchpadProfile);
    wl_list_insert(&touchpad->motion_filters, &accel->link);

    // Setup initial state
    touchpad->reset = 1;

    memset(touchpad->motion_history, 0, sizeof touchpad->motion_history);
    touchpad->motion_index = 0;
    touchpad->motion_count = 0;

    touchpad->state = TOUCHPAD_STATE_NONE;
    touchpad->last_finger_state = 0;
    touchpad->finger_state = 0;
}

int
WaylandEvdevInputEvent::handleInputEvent(int fd, uint32_t mask, void *data)
{
    WL_UNUSED(mask);

    struct evdev_input_device *device = (evdev_input_device*)data;
    struct input_event ev[32];
    int len;
    do {
        if (device->mtdev){
            len = mtdev_get(device->mtdev, fd, ev,
                ARRAY_LENGTH(ev)) * sizeof(struct input_event);
        }
        else {
            len = read(fd, &ev, sizeof(ev));
        }

        if (len < 0 || len % sizeof(ev[0]) != 0){
            return 1;
        }

        WaylandEvdevInputEvent::processEvents(device, ev, len / sizeof(ev[0]));

    } while (len > 0);

    return 1;
}

void
WaylandEvdevInputEvent::processEvents(struct evdev_input_device *device,
                                      struct input_event *ev,
                                      int count)
{
    struct evdev_dispatch *dispatch = device->dispatch;
    struct input_event *e, *end;
    uint32_t time = 0;

    device->pending_events = 0;

    e = ev;
    end = e + count;
    for (; e < end; ++e){
        time = e->time.tv_sec * 1000 + e->time.tv_usec / 1000;

        if (!isMotionEvent(e)){
            WaylandEvdevInputEvent::flushMotion(device, time);
        }

        dispatch->interface->process(dispatch, device, e, time);
    }

    WaylandEvdevInputEvent::flushMotion(device, time);
}

void
WaylandEvdevInputEvent::flushMotion(struct evdev_input_device *device,
                                    uint32_t time)
{
    if (!device->pending_events)
        return;

    WaylandEvdevInputEvent *inputEvent = static_cast<WaylandEvdevInputEvent*>(device->master);
    if (!inputEvent)
        return;

    if (device->pending_events & EVDEV_RELATIVE_MOTION){
        struct wl_seat *wlSeat = inputEvent->inputDevice().seat();
        if (wlSeat){
            // notify_motion
            notifyMotion(device, time,
                         wlSeat->pointer->x + device->rel.dx,
                         wlSeat->pointer->y + device->rel.dy);
        }
        device->pending_events &= ~EVDEV_RELATIVE_MOTION;
        device->rel.dx = 0;
        device->rel.dy = 0;
    }
    if (device->pending_events & EVDEV_ABSOLUTE_MT_DOWN){
        // notify_touch
        notifyTouch(device);
        device->pending_events &= ~EVDEV_ABSOLUTE_MT_DOWN;
        device->pending_events &= ~EVDEV_ABSOLUTE_MT_MOTION;
    }
    if (device->pending_events & EVDEV_ABSOLUTE_MT_MOTION){
        // notify_touch
        notifyTouch(device);
        device->pending_events &= ~EVDEV_ABSOLUTE_MT_DOWN;
        device->pending_events &= ~EVDEV_ABSOLUTE_MT_MOTION;
    }
    if (device->pending_events & EVDEV_ABSOLUTE_MT_UP){
        // notify_touch
        notifyTouch(device);
        device->pending_events &= ~EVDEV_ABSOLUTE_MT_UP;
    }
    if (device->pending_events & EVDEV_ABSOLUTE_MOTION){
        // notify_motion
        notifyMotion(device, time,
                     wl_fixed_from_int(device->abs.x),
                     wl_fixed_from_int(device->abs.y));
        device->pending_events &= ~EVDEV_ABSOLUTE_MOTION;
    }
}

/// Default event handler //////////////////////////////////////////////////

void
WaylandEvdevInputEvent::fallbackProcess(struct evdev_dispatch *dispatch,
                                        struct evdev_input_device *device,
                                        struct input_event *e,
                                        uint32_t time)
{
    WL_UNUSED(dispatch);

    switch (e->type){
    case EV_REL:
        evdevProcessRelative(device, time, e);
        break;
    case EV_ABS:
        evdevProcessAbsolute(device, e);
        break;
    case EV_KEY:
        evdevProcessKey(device, time, e);
        break;
    }
}

void
WaylandEvdevInputEvent::fallbackDestroy(struct evdev_dispatch *dispatch)
{
    if (dispatch) free(dispatch);
}

void
WaylandEvdevInputEvent::evdevProcessRelative(struct evdev_input_device *device,
                                             uint32_t /*time*/, struct input_event *e)
{
    switch (e->code){
    case REL_X:
        device->rel.dx += wl_fixed_from_int(e->value);
        device->pending_events |= EVDEV_RELATIVE_MOTION;
        break;
    case REL_Y:
        device->rel.dy += wl_fixed_from_int(e->value);
        device->pending_events |= EVDEV_RELATIVE_MOTION;
        break;
    case REL_WHEEL:
    case REL_HWHEEL:
        // not supported
        break;
    }
}

void
WaylandEvdevInputEvent::evdevProcessAbsolute(struct evdev_input_device *device,
                                             struct input_event *e)
{
    WaylandEvdevInputEvent *inputEvent = static_cast<WaylandEvdevInputEvent*>(device->master);
    if (!inputEvent)
        return;

    int w = inputEvent->m_screenWidth;
    int h = inputEvent->m_screenHeight;

    if (device->isMt){
        processTouch(device, e, w, h);
    } else {
        processAbsoluteMotion(device, e, w, h);
    }
}

void
WaylandEvdevInputEvent::evdevProcessKey(struct evdev_input_device *device,
                                        uint32_t time, struct input_event *e)
{
    if (e->value == 2)
        return;

    switch (e->code){
    case BTN_LEFT:
    case BTN_RIGHT:
    case BTN_MIDDLE:
    case BTN_SIDE:
    case BTN_EXTRA:
    case BTN_FORWARD:
    case BTN_BACK:
    case BTN_TASK:
    case BTN_TOUCH:
        // notify_button
        notifyButton(device, time, e->code,
                     e->value ? WL_POINTER_BUTTON_STATE_PRESSED
                              : WL_POINTER_BUTTON_STATE_RELEASED);
        break;
    default:
        // notify_key
        notifyKey(device, time, e->code,
                  e->value ? WL_KEYBOARD_KEY_STATE_PRESSED
                           : WL_KEYBOARD_KEY_STATE_RELEASED,
                  true);
        break;
    }
}

/// Multi-touch event handler //////////////////////////////////////////////

void
WaylandEvdevInputEvent::touchpadProcess(struct evdev_dispatch *dispatch,
                                        struct evdev_input_device *device,
                                        struct input_event *e,
                                        uint32_t time)
{
    struct touchpad_dispatch *touchpad = (struct touchpad_dispatch*)dispatch;

    switch (e->type){
    case EV_SYN:
        if (e->code == SYN_REPORT)
            touchpad->event_mask |= TOUCHPAD_EVENT_REPORT;
        break;
    case EV_ABS:
        touchpadProcessAbsolute(touchpad, device, e);
        break;
    case EV_KEY:
        touchpadProcessKey(touchpad, device, e, time);
        break;
    }

    touchpadUpdateState(touchpad, time);
}

void
WaylandEvdevInputEvent::touchpadDestroy(struct evdev_dispatch *dispatch)
{
    struct touchpad_dispatch *touchpad = (struct touchpad_dispatch*)dispatch;
    struct motion_filter *filter;
    struct motion_filter *next;

    wl_list_for_each_safe(filter, next, &touchpad->motion_filters, link){
        filter->interface->destroy(filter);
    }

    if (dispatch) free(dispatch);
}

void
WaylandEvdevInputEvent::touchpadProcessAbsolute(struct touchpad_dispatch *touchpad,
                                                struct evdev_input_device *device,
                                                struct input_event *e)
{
    WL_UNUSED(device);

    switch (e->code){
    case ABS_PRESSURE:
        if (e->value > touchpad->pressure.press)
            touchpad->state = TOUCHPAD_STATE_PRESS;
        else if (e->value > touchpad->pressure.touch_high)
            touchpad->state = TOUCHPAD_STATE_TOUCH;
        else if (e->value < touchpad->pressure.touch_low){
            if (touchpad->state > TOUCHPAD_STATE_NONE)
                touchpad->reset = 1;
            touchpad->state = TOUCHPAD_STATE_NONE;
        }
        break;
    case ABS_X:
        if (touchpad->state >= TOUCHPAD_STATE_TOUCH){
            touchpad->hw_abs.x = e->value;
            touchpad->event_mask |= TOUCHPAD_EVENT_ABSOLUTE_ANY;
            touchpad->event_mask |= TOUCHPAD_EVENT_ABSOLUTE_X;
        }
        break;
    case ABS_Y:
        if (touchpad->state >= TOUCHPAD_STATE_TOUCH){
            touchpad->hw_abs.y = e->value;
            touchpad->event_mask |= TOUCHPAD_EVENT_ABSOLUTE_ANY;
            touchpad->event_mask |= TOUCHPAD_EVENT_ABSOLUTE_Y;
        }
        break;
    }
}

void
WaylandEvdevInputEvent::touchpadProcessKey(struct touchpad_dispatch *touchpad,
                                           struct evdev_input_device * device,
                                           struct input_event *e,
                                           uint32_t time)
{
    switch (e->code){
    case BTN_TOUCH:
        if (!touchpad->has_pressure){
            if (!e->value){
                touchpad->state = TOUCHPAD_STATE_NONE;
                touchpad->reset = 1;
            }
            else {
                touchpad->state = e->value ? TOUCHPAD_STATE_TOUCH
                                           : TOUCHPAD_STATE_NONE;
            }
        }
        break;
    case BTN_LEFT:
    case BTN_RIGHT:
    case BTN_MIDDLE:
    case BTN_SIDE:
    case BTN_EXTRA:
    case BTN_FORWARD:
    case BTN_BACK:
    case BTN_TASK:
        // notify_button
        notifyButton(device, time, e->code,
                     e->value ? WL_POINTER_BUTTON_STATE_PRESSED
                              : WL_POINTER_BUTTON_STATE_RELEASED);
        break;
    case BTN_TOOL_PEN:
    case BTN_TOOL_RUBBER:
    case BTN_TOOL_BRUSH:
    case BTN_TOOL_PENCIL:
    case BTN_TOOL_AIRBRUSH:
    case BTN_TOOL_MOUSE:
    case BTN_TOOL_LENS:
        touchpad->reset = 1;
        break;
    case BTN_TOOL_FINGER:
        touchpad->finger_state =
            ~TOUCHPAD_FINGERS_ONE | e->value ? TOUCHPAD_FINGERS_ONE : 0;
        break;
    case BTN_TOOL_DOUBLETAP:
        touchpad->finger_state =
            ~TOUCHPAD_FINGERS_TWO | e->value ? TOUCHPAD_FINGERS_TWO : 0;
        break;
    case BTN_TOOL_TRIPLETAP:
        touchpad->finger_state =
            ~TOUCHPAD_FINGERS_THREE | e->value ? TOUCHPAD_FINGERS_THREE : 0;
        break;
    }
}

void
WaylandEvdevInputEvent::touchpadUpdateState(struct touchpad_dispatch *touchpad,
                                            uint32_t time)
{
    int motion_index;
    int center_x, center_y;
    double dx, dy;

    if (touchpad->reset ||
        touchpad->last_finger_state != touchpad->finger_state) {
        touchpad->reset = 0;
        touchpad->motion_count = 0;
        touchpad->event_mask = TOUCHPAD_EVENT_NONE;
        touchpad->event_mask_filter =
            TOUCHPAD_EVENT_ABSOLUTE_X | TOUCHPAD_EVENT_ABSOLUTE_Y;

        touchpad->last_finger_state = touchpad->finger_state;

        return;
    }
    touchpad->last_finger_state = touchpad->finger_state;

    if (!(touchpad->event_mask & TOUCHPAD_EVENT_REPORT))
        return;
    else
        touchpad->event_mask &= ~TOUCHPAD_EVENT_REPORT;

    if ((touchpad->event_mask & touchpad->event_mask_filter) !=
        touchpad->event_mask_filter)
        return;

    touchpad->event_mask_filter = TOUCHPAD_EVENT_ABSOLUTE_ANY;
    touchpad->event_mask = 0;

    // Avoid noice by moving center only when delta reaches a threshold
    // distance from the old center
    if (touchpad->motion_count > 0) {
        center_x = hysteresis(touchpad->hw_abs.x,
                      touchpad->hysteresis.center_x,
                      touchpad->hysteresis.margin_x);
        center_y = hysteresis(touchpad->hw_abs.y,
                      touchpad->hysteresis.center_y,
                      touchpad->hysteresis.margin_y);
    }
    else {
        center_x = touchpad->hw_abs.x;
        center_y = touchpad->hw_abs.y;
    }
    touchpad->hysteresis.center_x = center_x;
    touchpad->hysteresis.center_y = center_y;
    touchpad->hw_abs.x = center_x;
    touchpad->hw_abs.y = center_y;

    // Update motion history tracker
    motion_index = (touchpad->motion_index + 1) % TOUCHPAD_HISTORY_LENGTH;
    touchpad->motion_index = motion_index;
    touchpad->motion_history[motion_index].x = touchpad->hw_abs.x;
    touchpad->motion_history[motion_index].y = touchpad->hw_abs.y;
    if (touchpad->motion_count < 4)
        touchpad->motion_count++;

    if (touchpad->motion_count >= 4) {
        touchpadGetDelta(touchpad, &dx, &dy);

        filterMotion(touchpad, &dx, &dy, time);

        touchpad->device->rel.dx = wl_fixed_from_double(dx);
        touchpad->device->rel.dy = wl_fixed_from_double(dy);
        touchpad->device->pending_events |= EVDEV_RELATIVE_MOTION;
    }
}

/// Notifier ///////////////////////////////////////////////////////////////

void
WaylandEvdevInputEvent::notifyButton(struct evdev_input_device *device,
                                     uint32_t time, int32_t button,
                                     enum wl_pointer_button_state state)
{
    WLEvent         wlEvent;
    struct wl_seat *wlSeat = NULL;
    uint32_t        serial;

    WaylandEvdevInputEvent *inputEvent = static_cast<WaylandEvdevInputEvent*>(device->master);
    if (!inputEvent)
        return;

    wlSeat = inputEvent->inputDevice().seat();
    serial = wl_display_next_serial(inputEvent->inputDevice().display());

    if (state == WL_POINTER_BUTTON_STATE_PRESSED){
        if (wlSeat->pointer->button_count == 0){
            wlSeat->pointer->grab_button = button;
            wlSeat->pointer->grab_time = time;
            wlSeat->pointer->grab_x = wlSeat->pointer->x;
            wlSeat->pointer->grab_y = wlSeat->pointer->y;
        }
        ++wlSeat->pointer->button_count;
    }
    else {
        --wlSeat->pointer->button_count;
    }
    wlEvent.x = wl_fixed_to_int(wlSeat->pointer->x);
    wlEvent.y = wl_fixed_to_int(wlSeat->pointer->y);
    wlEvent.buttonState = state;
    wlEvent.serial = serial;

    inputEvent->windowSystem().manageWLInputEvent(INPUT_DEVICE_POINTER,
        state == WL_POINTER_BUTTON_STATE_PRESSED ? INPUT_STATE_PRESSED :
        INPUT_STATE_RELEASED, &wlEvent);

    if (wlSeat->pointer->button_count == 1){
        wlSeat->pointer->grab_serial =
            wl_display_get_serial(inputEvent->inputDevice().display());
    }
}

void
WaylandEvdevInputEvent::notifyMotion(struct evdev_input_device *device,
                                     uint32_t time,
                                     wl_fixed_t fx, wl_fixed_t fy)
{
    WL_UNUSED(time);

    WLEvent         wlEvent;
    struct wl_seat *wlSeat = NULL;
    int             x, y;
    //int             old_x, old_y;
    int             w, h;

    WaylandEvdevInputEvent *inputEvent = static_cast<WaylandEvdevInputEvent*>(device->master);
    if (!inputEvent)
        return;

    wlSeat = inputEvent->inputDevice().seat();
    w = inputEvent->m_screenWidth;
    h = inputEvent->m_screenHeight;

    x = wl_fixed_to_int(fx);
    y = wl_fixed_to_int(fy);
    //old_x = wl_fixed_to_int(wlSeat->pointer->x);
    //old_y = wl_fixed_to_int(wlSeat->pointer->y);
    if (x < 0) x = 0;
    if (x > w) x = w;
    if (y < 0) y = 0;
    if (y > h) y = h;

    wlSeat->pointer->x = wl_fixed_from_int(x);
    wlSeat->pointer->y = wl_fixed_from_int(y);

    wlEvent.x = x;
    wlEvent.y = y;

    inputEvent->windowSystem().manageWLInputEvent(
        INPUT_DEVICE_POINTER, INPUT_STATE_MOTION, &wlEvent);
}

void
WaylandEvdevInputEvent::notifyKey(struct evdev_input_device *device,
                                  uint32_t time, uint32_t key,
                                  enum wl_keyboard_key_state state,
                                  bool bUpdateAutomatic)
{
    WL_UNUSED(bUpdateAutomatic);

    WLEvent wlEvent;
    struct wl_seat *wlSeat = NULL;
    uint32_t *k, *end;

    WaylandEvdevInputEvent *inputEvent = static_cast<WaylandEvdevInputEvent*>(device->master);
    if (!inputEvent)
        return;

    wlSeat = inputEvent->inputDevice().seat();
    if (state == WL_KEYBOARD_KEY_STATE_PRESSED){
        wlSeat->keyboard->grab_key = key;
        wlSeat->keyboard->grab_time = time;
    }

    end = (uint32_t*)(((unsigned char*)wlSeat->keyboard->keys.data) + wlSeat->keyboard->keys.size);
    for (k = (uint32_t*)wlSeat->keyboard->keys.data; k < end; ++k){
        if (*k == key){
            // Ignore server-generated repeats
            if (state == WL_KEYBOARD_KEY_STATE_PRESSED)
                return;
            *k = *--end;
        }
    }
    wlSeat->keyboard->keys.size = end - (uint32_t*)wlSeat->keyboard->keys.data;
    if (state == WL_KEYBOARD_KEY_STATE_PRESSED){
        k = (uint32_t*)wl_array_add(&wlSeat->keyboard->keys, sizeof(*k));
        *k = key;
    }

    wlEvent.keyCode = key;
    wlEvent.keyState = state;

    inputEvent->windowSystem().manageWLInputEvent(INPUT_DEVICE_KEYBOARD,
        state == WL_KEYBOARD_KEY_STATE_PRESSED ? INPUT_STATE_PRESSED
                                               : INPUT_STATE_RELEASED, &wlEvent);
}

void
WaylandEvdevInputEvent::notifyTouch(struct evdev_input_device *device)
{
    WLEvent         wlEvent;
    InputEventState eventState = INPUT_STATE_OTHER;

    WaylandEvdevInputEvent *inputEvent = static_cast<WaylandEvdevInputEvent*>(device->master);
    if (!inputEvent)
        return;

    if (device->pending_events & EVDEV_ABSOLUTE_MT_DOWN){
        wlEvent.x = (int)wl_fixed_from_int(device->mt.x[device->mt.slot]);
        wlEvent.y = (int)wl_fixed_from_int(device->mt.y[device->mt.slot]);
        wlEvent.touchId = device->mt.slot;
        wlEvent.touchType = WL_TOUCH_DOWN;
        eventState = INPUT_STATE_PRESSED;
    } else
    if (device->pending_events & EVDEV_ABSOLUTE_MT_MOTION){
        wlEvent.x = (int)wl_fixed_from_int(device->mt.x[device->mt.slot]);
        wlEvent.y = (int)wl_fixed_from_int(device->mt.y[device->mt.slot]);
        wlEvent.touchId = device->mt.slot;
        wlEvent.touchType = WL_TOUCH_MOTION;
        eventState = INPUT_STATE_MOTION;
    } else
    if (device->pending_events & EVDEV_ABSOLUTE_MT_UP){
        wlEvent.x = 0;
        wlEvent.y = 0;
        wlEvent.touchId = device->mt.slot;
        wlEvent.touchType = WL_TOUCH_UP;
        eventState = INPUT_STATE_RELEASED;
    }
    else {
        return;
    }

    inputEvent->windowSystem().manageWLInputEvent(INPUT_DEVICE_TOUCH, eventState, &wlEvent);
}

void
WaylandEvdevInputEvent::notifyModifiers(struct wl_seat *wlSeat, uint32_t serial)
{
    uint32_t mods_depressed, mods_latched, mods_locked, group;
    uint32_t mods_lookup;
    int changed = 0;

    mods_depressed = xkb_state_serialize_mods(m_xkbState.state, (xkb_state_component)XKB_STATE_DEPRESSED);
    mods_latched   = xkb_state_serialize_mods(m_xkbState.state, (xkb_state_component)XKB_STATE_LATCHED);
    mods_locked    = xkb_state_serialize_mods(m_xkbState.state, (xkb_state_component)XKB_STATE_LOCKED);
    group          = xkb_state_serialize_mods(m_xkbState.state, (xkb_state_component)XKB_STATE_EFFECTIVE);

    if (mods_depressed != wlSeat->keyboard->modifiers.mods_depressed ||
        mods_latched   != wlSeat->keyboard->modifiers.mods_latched   ||
        mods_locked    != wlSeat->keyboard->modifiers.mods_locked    ||
        group          != wlSeat->keyboard->modifiers.group){
        changed = 1;
    }

    wlSeat->keyboard->modifiers.mods_depressed = mods_depressed;
    wlSeat->keyboard->modifiers.mods_latched   = mods_latched;
    wlSeat->keyboard->modifiers.mods_locked    = mods_locked;
    wlSeat->keyboard->modifiers.group          = group;

    // And update the modifier_state for bindings
    mods_lookup = mods_depressed | mods_latched;
    m_modifierState = 0;
    if (mods_lookup & (1 << m_xkbInfo.ctrl_mod))  m_modifierState |= MODIFIER_CTRL;
    if (mods_lookup & (1 << m_xkbInfo.alt_mod))   m_modifierState |= MODIFIER_ALT;
    if (mods_lookup & (1 << m_xkbInfo.super_mod)) m_modifierState |= MODIFIER_SUPER;
    if (mods_lookup & (1 << m_xkbInfo.shift_mod)) m_modifierState |= MODIFIER_SHIFT;

    if (changed){
        m_inputDevice->sendModifiers(serial);
    }
}
