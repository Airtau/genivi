/***************************************************************************
 *
 * Copyright 2010,2011 BMW Car IT GmbH
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
 ****************************************************************************/
#include "OpenGLES2App.h"
#include "LayerScene.h"
#include <ilm_client.h>

#include <iostream>
using std::cout;
using std::endl;

#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <sys/time.h>

#define RUNTIME_IN_MS() (GetTickCount() - startTimeInMS)

OpenGLES2App::OpenGLES2App(float fps, float animationSpeed, SurfaceConfiguration* config)
: m_framesPerSecond(fps)
, m_animationSpeed(animationSpeed)
, m_timerIntervalInMs(1000.0 / m_framesPerSecond)
{
    ilm_init();
    createX11Context(config);
    createEGLContext();
    setupLayerMangement(config);

    if (config->nosky)
    {
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    }
    else
    {
        glClearColor(0.2f, 0.2f, 0.5f, 1.0f);
    }
    glDisable(GL_BLEND);

    glClearDepthf(1.0f);
    glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
}

OpenGLES2App::~OpenGLES2App()
{
    destroyEglContext();
    destroyX11Context();
    ilm_destroy();
}

void OpenGLES2App::mainloop()
{
    unsigned int startTimeInMS = GetTickCount();
    unsigned int frameStartTimeInMS = 0;
    unsigned int renderTimeInMS = 0;
    unsigned int frameEndTimeInMS = 0;
    unsigned int frameTimeInMS = 0;

    while (true)
    {
        frameTimeInMS = frameEndTimeInMS - frameStartTimeInMS;
        frameStartTimeInMS = RUNTIME_IN_MS();

        update(m_animationSpeed * frameStartTimeInMS, m_animationSpeed * frameTimeInMS);
        render();
        swapBuffers();

        renderTimeInMS = RUNTIME_IN_MS() - frameStartTimeInMS;

        if (renderTimeInMS < m_timerIntervalInMs)
        {
            usleep((m_timerIntervalInMs - renderTimeInMS) * 1000);
        }

        frameEndTimeInMS = RUNTIME_IN_MS();
    }
}

bool OpenGLES2App::createX11Context(SurfaceConfiguration* config)
{
    int width = config->surfaceWidth;
    int height = config->surfaceHeight;

    t_ilm_bool result = ILM_TRUE;
    Window rootWindow;
    XSetWindowAttributes windowAttributes;
    unsigned int windowMask;
    int colorDepth;
    int widthCorrected, heightCorrected;

    m_x11ContextStruct.x11Window = 0;
    m_x11ContextStruct.x11Display = NULL;
    m_x11ContextStruct.x11Screen = 0;
    m_x11ContextStruct.x11Visual = NULL;

    m_x11ContextStruct.x11Display = XOpenDisplay(0);
    if (!m_x11ContextStruct.x11Display)
    {
        cout << "Error: Unable to open X display\n";
        return ILM_FALSE;
    }
    m_x11ContextStruct.x11Screen = XDefaultScreen(m_x11ContextStruct.x11Display);

    // Get the root window parameters
    rootWindow = RootWindow(m_x11ContextStruct.x11Display, m_x11ContextStruct.x11Screen);
    colorDepth = 32; // TODO: DefaultDepth(m_x11ContextStruct.x11Display, m_x11ContextStruct.x11Screen);

    // Alloc memory for the visual info
    m_x11ContextStruct.x11Visual = (XVisualInfo*) malloc(sizeof(XVisualInfo));

    // Try to find a visual which is matching the needed parameters
    if (!XMatchVisualInfo(m_x11ContextStruct.x11Display,
            m_x11ContextStruct.x11Screen, colorDepth, TrueColor,
            m_x11ContextStruct.x11Visual))
    {
        cout << "Error: Unable to acquire visual\n";
        destroyX11Context();
        return ILM_FALSE;
    }

    // Create the rendercontext color map
    m_x11ContextStruct.x11Colormap = XCreateColormap(
            m_x11ContextStruct.x11Display, rootWindow,
            m_x11ContextStruct.x11Visual->visual, AllocNone);
    windowAttributes.colormap = m_x11ContextStruct.x11Colormap;

    // Add to these for handling other events
    windowAttributes.event_mask = StructureNotifyMask | ExposureMask
            | ButtonPressMask | ButtonReleaseMask | KeyPressMask
            | KeyReleaseMask;
    windowAttributes.backing_store = Always;

    // Set the window mask attributes
    windowMask = CWBackPixel | CWBorderPixel | CWEventMask | CWColormap
            | CWBackingStore;

    // get the corrected window dimensions
    widthCorrected = width < XDisplayWidth(m_x11ContextStruct.x11Display,
            m_x11ContextStruct.x11Screen) ? width : XDisplayWidth(
            m_x11ContextStruct.x11Display, m_x11ContextStruct.x11Screen);
    heightCorrected = height < XDisplayHeight(m_x11ContextStruct.x11Display,
            m_x11ContextStruct.x11Screen) ? height : XDisplayHeight(
            m_x11ContextStruct.x11Display, m_x11ContextStruct.x11Screen);

    // Creates the X11 window
    m_x11ContextStruct.x11Window
            = XCreateWindow(
                    m_x11ContextStruct.x11Display,
                    RootWindow(m_x11ContextStruct.x11Display, m_x11ContextStruct.x11Screen),
                    0, 0, widthCorrected, heightCorrected, 0,
                    m_x11ContextStruct.x11Visual->depth, InputOutput,
                    m_x11ContextStruct.x11Visual->visual, windowMask,
                    &windowAttributes);

    // map the window
    XMapWindow(m_x11ContextStruct.x11Display, m_x11ContextStruct.x11Window);
    XFlush(m_x11ContextStruct.x11Display);
    return result;
}

bool OpenGLES2App::createEGLContext()
{
    t_ilm_bool result = ILM_TRUE;
    m_eglContextStruct.eglDisplay = NULL;
    m_eglContextStruct.eglSurface = NULL;
    m_eglContextStruct.eglContext = NULL;

    m_eglContextStruct.eglDisplay = eglGetDisplay((EGLNativeDisplayType) m_x11ContextStruct.x11Display); // TODO: remove all C style casts in C++ code; use C++ casts
    EGLint eglstatus = eglGetError();
    if (!m_eglContextStruct.eglDisplay)
    {
        cout << "Error: eglGetDisplay() failed.\n";
    }

    EGLint iMajorVersion, iMinorVersion;
    if (!eglInitialize(m_eglContextStruct.eglDisplay, &iMajorVersion,
            &iMinorVersion))
    {
        cout << "Error: eglInitialize() failed.\n";
    }
    eglBindAPI(EGL_OPENGL_ES_API);
    eglstatus = eglGetError();
    if (eglstatus != EGL_SUCCESS)
    {
        cout << "Error: eglBindAPI() failed.\n";
    }

    EGLint pi32ConfigAttribs[] = { EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_RENDERABLE_TYPE,
                                   EGL_OPENGL_ES2_BIT, EGL_RED_SIZE, 8, EGL_ALPHA_SIZE, 8, EGL_DEPTH_SIZE, 1, EGL_NONE };
    int iConfigs;

    if (!eglChooseConfig(m_eglContextStruct.eglDisplay, pi32ConfigAttribs, &m_eglContextStruct.eglConfig, 1, &iConfigs) || (iConfigs != 1))
    {
        cout << "Error: eglChooseConfig() failed.\n";
    }

    m_eglContextStruct.eglSurface = eglCreateWindowSurface(
            m_eglContextStruct.eglDisplay, m_eglContextStruct.eglConfig,
            (EGLNativeWindowType) m_x11ContextStruct.x11Window, NULL);
    eglstatus = eglGetError();

    if (eglstatus != EGL_SUCCESS)
    {
        cout << "Error: eglCreateWindowSurface() failed.\n";
    }

    EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };

    m_eglContextStruct.eglContext = eglCreateContext(
            m_eglContextStruct.eglDisplay, m_eglContextStruct.eglConfig, NULL,
            contextAttribs);

    eglstatus = eglGetError();
    if (eglstatus != EGL_SUCCESS)
    {
        cout << "Error: eglCreateContext() failed.\n";
    }

    eglMakeCurrent(m_eglContextStruct.eglDisplay,
            m_eglContextStruct.eglSurface, m_eglContextStruct.eglSurface,
            m_eglContextStruct.eglContext);
    eglSwapInterval(m_eglContextStruct.eglDisplay, 1);
    eglstatus = eglGetError();
    if (eglstatus != EGL_SUCCESS)
    {
        cout << "Error: eglMakeCurrent() failed.\n";
    }

    return result;
}

bool OpenGLES2App::setupLayerMangement(SurfaceConfiguration* config)
{
    // register surfaces to layermanager
    t_ilm_layer layerid = (t_ilm_layer)config->layerId;//LAYER_EXAMPLE_GLES_APPLICATIONS;
    t_ilm_surface surfaceid = (t_ilm_surface)config->surfaceId;//SURFACE_EXAMPLE_EGLX11_APPLICATION;
    int width = config->surfaceWidth;
    int height = config->surfaceHeight;
    int posX = config->surfacePosX;
    int posY = config->surfacePosY;
    float opacity = config->opacity;

    cout << "creating surface " << surfaceid << "\n";
    ilm_surfaceCreate( (t_ilm_nativehandle) m_x11ContextStruct.x11Window, width, height,
            ILM_PIXELFORMAT_RGBA_8888, &surfaceid);

    cout << "set surface " << surfaceid << " dest region " << posX << ", " << posY << ", " << width << ", " << height << "\n";
    ilm_surfaceSetDestinationRectangle(surfaceid, posX, posY, width, height);

    cout << "set surface " << surfaceid << " src region " << 0 << ", " << 0 << ", " << width << ", " << height << "\n";
    ilm_surfaceSetSourceRectangle(surfaceid, 0, 0, width, height);

    cout << "Set surface " << surfaceid << " visible\n";
    ilm_surfaceSetVisibility(surfaceid, ILM_TRUE);

    cout << "Set surface " << surfaceid << " opacity " << opacity << "\n";
    ilm_surfaceSetOpacity(surfaceid, opacity);

    cout << "add surface " << surfaceid << " to layer " << layerid << "\n";
    ilm_layerAddSurface(layerid, surfaceid);

    cout << "commit\n";
    return (ILM_SUCCESS == ilm_commitChanges()) ? true : false;
}

void OpenGLES2App::destroyEglContext()
{
    if (m_eglContextStruct.eglDisplay != NULL)
    {
        eglMakeCurrent(m_eglContextStruct.eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglTerminate(m_eglContextStruct.eglDisplay);
    }
}

void OpenGLES2App::destroyX11Context()
{
    if (m_x11ContextStruct.x11Window)
    {
        XDestroyWindow(m_x11ContextStruct.x11Display, m_x11ContextStruct.x11Window);
    }

    if (m_x11ContextStruct.x11Colormap)
    {
        XFreeColormap(m_x11ContextStruct.x11Display, m_x11ContextStruct.x11Colormap);
    }

    if (m_x11ContextStruct.x11Display)
    {
        XCloseDisplay(m_x11ContextStruct.x11Display);
    }

    if (m_x11ContextStruct.x11Visual)
    {
        free(m_x11ContextStruct.x11Visual);
    }
}

unsigned int OpenGLES2App::GetTickCount()
{
    struct timeval ts;
    gettimeofday(&ts, 0);
    return (t_ilm_uint) (ts.tv_sec * 1000 + (ts.tv_usec / 1000));
}

void OpenGLES2App::swapBuffers()
{
    eglSwapBuffers(m_eglContextStruct.eglDisplay, m_eglContextStruct.eglSurface);
}
