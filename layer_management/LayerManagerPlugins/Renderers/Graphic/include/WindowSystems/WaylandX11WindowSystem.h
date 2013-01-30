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

#ifndef _WAYLANDX11WINDOWSYSTEM_H_
#define _WAYLANDX11WINDOWSYSTEM_H_
#include "WindowSystems/WaylandBaseWindowSystem.h"
#include "X11/Xlib.h"
#include <X11/Xutil.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

class WaylandX11WindowSystem: public WaylandBaseWindowSystem
{
public:
    WaylandX11WindowSystem(const char* displayname, int width, int height, Scene* pScene, InputManager* pInputManager);
#if 0
    virtual bool init(BaseGraphicSystem<EGLNativeDisplayType, EGLNativeWindowType>* sys);
#endif
    Display* x11Display() const;

protected:
    virtual bool createNativeContext();
    virtual bool initGraphicSystem();
    virtual bool createInputEvent();

private:
#if 0
    static void* eventLoopCallback(void* ptr);
#endif

    Window m_x11Window;
    Display* m_x11Display;
    long m_x11Screen;
    XVisualInfo* m_x11Visual;
    Colormap m_x11Colormap;
};

inline Display* WaylandX11WindowSystem::x11Display() const { return m_x11Display; }

#endif /* _WAYLANDX11WINDOWSYSTEM_H_ */
