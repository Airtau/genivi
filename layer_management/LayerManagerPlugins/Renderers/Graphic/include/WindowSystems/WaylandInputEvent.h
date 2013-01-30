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

#ifndef _WAYLANDINPUTEVENT_H_
#define _WAYLANDINPUTEVENT_H_
#include <wayland-server.h>
#include <xkbcommon/xkbcommon.h>
#include "Log.h"
#include "config.h"
#include "WindowSystems/WaylandInputDevice.h"

enum keyboard_modifier {
    MODIFIER_CTRL  = (1 << 0),
    MODIFIER_ALT   = (1 << 1),
    MODIFIER_SUPER = (1 << 2),
    MODIFIER_SHIFT = (1 << 3)
};

enum keyboard_led {
    LED_NUM_LOCK    = (1 << 0),
    LED_CAPS_LOCK   = (1 << 1),
    LED_SCROLL_LOCK = (1 << 2)
};

class WaylandBaseWindowSystem;

//////////////////////////////////////////////////////////////////////////////
class WaylandInputEvent
{
// Properties
protected:
    WaylandBaseWindowSystem *m_windowSystem;
    WaylandInputDevice      *m_inputDevice;
    struct wl_event_source  *m_wlEventSource;
    int m_fd;

    struct weston_xkb_info {
        struct xkb_keymap *keymap;
        int keymap_fd;
        size_t keymap_size;
        char *keymap_area;
        xkb_mod_index_t shift_mod;
        xkb_mod_index_t caps_mod;
        xkb_mod_index_t ctrl_mod;
        xkb_mod_index_t alt_mod;
        xkb_mod_index_t mod2_mod;
        xkb_mod_index_t mod3_mod;
        xkb_mod_index_t super_mod;
        xkb_mod_index_t mod5_mod;
        xkb_led_index_t num_led;
        xkb_led_index_t caps_led;
        xkb_led_index_t scroll_led;
    } m_xkbInfo;

    struct weston_xkb_state {
        struct xkb_state *state;
        enum keyboard_led leds;
    } m_xkbState;

    int m_modifierState;

    struct xkb_context    *m_xkbContext;
    struct xkb_rule_names  m_xkbNames;

// Methods
public:
    WaylandInputEvent(WaylandBaseWindowSystem *windowSystem);
    virtual ~WaylandInputEvent();

    virtual void setupInputEvent();

    WaylandInputDevice& inputDevice() const;
    WaylandBaseWindowSystem& windowSystem() const;

protected:
    int createAnonymousFile(off_t size);
    void initPointerDevice();
    void initKeyboardDevice(struct xkb_keymap *keymap);
    void initTouchDevice();

private:
    void initInputEvent();
    void createNewKeymap();
    void buildGlobalKeymap();
};

inline WaylandInputDevice& WaylandInputEvent::inputDevice() const { return *m_inputDevice; }
inline WaylandBaseWindowSystem& WaylandInputEvent::windowSystem() const { return *m_windowSystem; }

#endif /* _WAYLANDINPUTEVENT_H_ */
