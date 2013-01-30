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

#include "WindowSystems/WaylandX11WindowSystem.h"
#include "Log.h"
#include "Layer.h"
#include <time.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <linux/fb.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <iomanip>

#include "GraphicSystems/GLESGraphicSystem.h"

#include "WindowSystems/WaylandServerinfoServerProtocol.h"
#include "WindowSystems/WaylandX11InputEvent.h"

WaylandX11WindowSystem::WaylandX11WindowSystem(const char* displayname, int width, int height, Scene* pScene, InputManager* pInputManager)
: WaylandBaseWindowSystem(displayname, width, height, pScene, pInputManager)
, m_x11Window(0)
, m_x11Display(0)
, m_x11Screen(0)
, m_x11Visual(0)
, m_x11Colormap(0)
{
    LOG_DEBUG("WaylandX11WindowSystem", "creating WaylandX11WindowSystem width:" << width << " height:" << height);
}

bool WaylandX11WindowSystem::initGraphicSystem()
{
    bool status = true;

    LOG_DEBUG("WaylandX11WindowSystem", "initGraphicSystem IN");

    //. create egl context
    status = graphicSystem->init((void*)m_x11Display, (void*)m_x11Window);
    if (false == status)
    {
        LOG_ERROR("WaylandX11WindowSystem", "failed to init graphicSystem");
        return false;
    }
    LOG_DEBUG("WaylandX11WindowSystem", "SUCCESS:init GraphicSystem");

    //. TODO: should not be here. graphicSystem should support.
    EGLDisplay eglDisplayhandle = ((GLESGraphicsystem*)(graphicSystem))->getEGLDisplay();
    PFNEGLBINDWAYLANDDISPLAYWL eglBindWaylandDisplayWL = (PFNEGLBINDWAYLANDDISPLAYWL)eglGetProcAddress("eglBindWaylandDisplayWL");
    if( eglBindWaylandDisplayWL != NULL )
    {
        eglBindWaylandDisplayWL( eglDisplayhandle, m_wlDisplay );
    }
    else
    {
        LOG_ERROR("WaylandX11WindowSystem", "Failed to get address of eglBindWaylandDisplayWL");
    }

    LOG_DEBUG("WaylandX11WindowSystem", "initGraphicSystem OUT");
    return true;
}

bool WaylandX11WindowSystem::createNativeContext()
{
    bool result = true;
    Window rootWindow = 0;
    XSetWindowAttributes windowAttributes;
    unsigned int windowMask = 0;
    int colorDepth = 0;
    int widthCorrected = 0;
    int heightCorrected = 0;

    LOG_DEBUG("WaylandX11WindowSystem", "createNativeContext IN");

    m_x11Window = 0;
    m_x11Display = NULL;
    m_x11Screen = 0;
    m_x11Visual = NULL;

    m_x11Display = XOpenDisplay(0);
    if (!m_x11Display)
    {
        printf("Error: Unable to open X display\n");
        return false;
    }
    m_x11Screen = XDefaultScreen(m_x11Display);

    // Get the root window parameters
    rootWindow = RootWindow(m_x11Display, m_x11Screen);
    colorDepth = 32; // TODO: DefaultDepth(m_x11Display, m_x11Screen);

    // Alloc memory for the visual info
    m_x11Visual = (XVisualInfo*)malloc(sizeof(XVisualInfo));

    // Try to find a visual which is matching the needed parameters
    if (!XMatchVisualInfo(m_x11Display,
            m_x11Screen, colorDepth, TrueColor,
            m_x11Visual))
    {
        printf("Error: Unable to acquire visual\n");
        //destroyX11Context();
        return false;
    }

    // Create the rendercontext color map
    m_x11Colormap = XCreateColormap(
            m_x11Display, rootWindow,
            m_x11Visual->visual, AllocNone);
    windowAttributes.colormap = m_x11Colormap;

    // Add to these for handling other events
    windowAttributes.event_mask = StructureNotifyMask | ExposureMask
            | ButtonPressMask | ButtonReleaseMask | KeyPressMask
            | KeyReleaseMask | Button1MotionMask | PointerMotionMask | FocusChangeMask;
    windowAttributes.backing_store = Always;

    // Set the window mask attributes
    windowMask = CWBackPixel | CWBorderPixel | CWEventMask | CWColormap
            | CWBackingStore;

    // get the corrected window dimensions
    widthCorrected = m_width < XDisplayWidth(m_x11Display,
            m_x11Screen) ? m_width : XDisplayWidth(
            m_x11Display, m_x11Screen);
    heightCorrected = m_height < XDisplayHeight(m_x11Display,
            m_x11Screen) ? m_height : XDisplayHeight(
            m_x11Display, m_x11Screen);

    // Creates the X11 window
    m_x11Window
            = XCreateWindow(
                    m_x11Display,
                    RootWindow(m_x11Display, m_x11Screen),
                    0, 0, widthCorrected, heightCorrected, 0,
                    m_x11Visual->depth, InputOutput,
                    m_x11Visual->visual, windowMask,
                    &windowAttributes);

    // map the window
    XMapWindow(m_x11Display, m_x11Window);
    XFlush(m_x11Display);

    LOG_DEBUG("WaylandX11WindowSystem", "createNativeContext OUT");

    return result;
}

bool WaylandX11WindowSystem::createInputEvent()
{
    m_inputEvent = new WaylandX11InputEvent(this);
    m_inputEvent->setupInputEvent();
    return true;
}
