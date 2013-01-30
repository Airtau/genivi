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

#ifndef _WAYLANDX11INPUTEVENT_H_
#define _WAYLANDX11INPUTEVENT_H_

#include "config.h"

//#define HAVE_XCB_POLL_FOR_QUEUED_EVENT 1

#include <wayland-server.h>
#include <xcb/xcb.h>
#ifdef HAVE_XCB_XKB // XCB-XKB Extension: not required
  #define explicit tmpexplicit
  #include <xcb/xkb.h>
  #undef explicit
#endif
#include <xkbcommon/xkbcommon.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xlib-xcb.h>

#include "Log.h"
#include "WindowSystems/WaylandBaseWindowSystem.h"

//////////////////////////////////////////////////////////////////////////////
extern "C"
{
    struct atom_ {
        xcb_atom_t xkbNames;
        xcb_atom_t string;
    };
}

//////////////////////////////////////////////////////////////////////////////
class WaylandX11InputEvent : public WaylandInputEvent
{
// Properties
private:
    Display          *m_x11Display;
    xcb_connection_t *m_xcbConn;
    xcb_screen_t     *m_xcbScreen;

    bool              m_hasXkb;
    uint8_t           m_xkbEventBase;
    struct atom_      m_atom;

// Methods
public:
    WaylandX11InputEvent(WaylandBaseWindowSystem *windowSystem);
    virtual ~WaylandX11InputEvent();

    virtual void setupInputEvent();

protected:
private:
    static int handleInputEvent(int fd, uint32_t mask, void *data);

    void setupXkb();
    bool hasXkb();
    struct xkb_keymap *getKeymap();

    xcb_connection_t *connection();
    uint8_t xkbEventBase();
};

inline bool WaylandX11InputEvent::hasXkb() { return m_hasXkb; }
inline xcb_connection_t* WaylandX11InputEvent::connection() { return m_xcbConn; }
inline uint8_t WaylandX11InputEvent::xkbEventBase() { return m_xkbEventBase; }

#endif /* _WAYLANDX11INPUTEVENT_H_ */
