/**************************************************************************
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
#include "egl_helper.h"
#include "LayerScene.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

typedef struct t_eglContextStruct
{
    EGLDisplay eglDisplay;
    EGLConfig eglConfig;
    EGLSurface eglSurface;
    EGLContext eglContext;
} EglContextStruct;

typedef struct t_x11ContextStruct
{
    Window x11Window;
    Display* x11Display;
    long x11Screen;
    XVisualInfo* x11Visual;
    Colormap x11Colormap;
} X11ContextStruct;

static EglContextStruct g_eglContextStruct;
static X11ContextStruct g_x11ContextStruct;

EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };

t_ilm_bool createX11Context(t_ilm_int width, t_ilm_int height)
{
    t_ilm_bool result = ILM_TRUE;
    Window rootWindow;
    XSetWindowAttributes windowAttributes;
    unsigned int windowMask;
    int colorDepth;
    int widthCorrected, heightCorrected;

    g_x11ContextStruct.x11Window = 0;
    g_x11ContextStruct.x11Display = NULL;
    g_x11ContextStruct.x11Screen = 0;
    g_x11ContextStruct.x11Visual = NULL;

    g_x11ContextStruct.x11Display = XOpenDisplay(0);
    if (!g_x11ContextStruct.x11Display)
    {
        printf("Error: Unable to open X display\n");
        return ILM_FALSE;
    }
    g_x11ContextStruct.x11Screen = XDefaultScreen(g_x11ContextStruct.x11Display);

    // Get the root window parameters
    rootWindow = RootWindow(g_x11ContextStruct.x11Display, g_x11ContextStruct.x11Screen);
    colorDepth = 32; // TODO: DefaultDepth(g_x11ContextStruct.x11Display, g_x11ContextStruct.x11Screen);

    // Alloc memory for the visual info
    g_x11ContextStruct.x11Visual = (XVisualInfo*) malloc(sizeof(XVisualInfo));

    // Try to find a visual which is matching the needed parameters
    if (!XMatchVisualInfo(g_x11ContextStruct.x11Display,
            g_x11ContextStruct.x11Screen, colorDepth, TrueColor,
            g_x11ContextStruct.x11Visual))
    {
        printf("Error: Unable to acquire visual\n");
        destroyX11Context();
        return ILM_FALSE;
    }

    // Create the rendercontext color map
    g_x11ContextStruct.x11Colormap = XCreateColormap(
            g_x11ContextStruct.x11Display, rootWindow,
            g_x11ContextStruct.x11Visual->visual, AllocNone);
    windowAttributes.colormap = g_x11ContextStruct.x11Colormap;

    // Add to these for handling other events
    windowAttributes.event_mask = StructureNotifyMask | ExposureMask
            | ButtonPressMask | ButtonReleaseMask | KeyPressMask
            | KeyReleaseMask;
    windowAttributes.backing_store = Always;

    // Set the window mask attributes
    windowMask = CWBackPixel | CWBorderPixel | CWEventMask | CWColormap
            | CWBackingStore;

    // get the corrected window dimensions
    widthCorrected = width < XDisplayWidth(g_x11ContextStruct.x11Display,
            g_x11ContextStruct.x11Screen) ? width : XDisplayWidth(
            g_x11ContextStruct.x11Display, g_x11ContextStruct.x11Screen);
    heightCorrected = height < XDisplayHeight(g_x11ContextStruct.x11Display,
            g_x11ContextStruct.x11Screen) ? height : XDisplayHeight(
            g_x11ContextStruct.x11Display, g_x11ContextStruct.x11Screen);

    // Creates the X11 window
    g_x11ContextStruct.x11Window
            = XCreateWindow(
                    g_x11ContextStruct.x11Display,
                    RootWindow(g_x11ContextStruct.x11Display, g_x11ContextStruct.x11Screen),
                    0, 0, widthCorrected, heightCorrected, 0,
                    g_x11ContextStruct.x11Visual->depth, InputOutput,
                    g_x11ContextStruct.x11Visual->visual, windowMask,
                    &windowAttributes);

    // map the window
    XMapWindow(g_x11ContextStruct.x11Display, g_x11ContextStruct.x11Window);
    XFlush(g_x11ContextStruct.x11Display);
    return result;
}

t_ilm_bool createEGLContext(t_ilm_int width, t_ilm_int height)
{
    g_eglContextStruct.eglDisplay = NULL;
    g_eglContextStruct.eglSurface = NULL;
    g_eglContextStruct.eglContext = NULL;

    g_eglContextStruct.eglDisplay = eglGetDisplay((EGLNativeDisplayType) g_x11ContextStruct.x11Display); // TODO: remove all C style casts in C++ code; use C++ casts
    EGLint eglstatus = eglGetError();
    if (!g_eglContextStruct.eglDisplay)
    {
        printf("Error: eglGetDisplay() failed.\n");
    }

    EGLint iMajorVersion, iMinorVersion;
    if (!eglInitialize(g_eglContextStruct.eglDisplay, &iMajorVersion,
            &iMinorVersion))
    {
        printf("Error: eglInitialize() failed.\n");
    }
    eglBindAPI(EGL_OPENGL_ES_API);
    eglstatus = eglGetError();
    if (eglstatus != EGL_SUCCESS)
    {
        printf("Error: eglBindAPI() failed.\n");
    }

    EGLint pi32ConfigAttribs[] = { EGL_SURFACE_TYPE, EGL_WINDOW_BIT, EGL_RENDERABLE_TYPE,
                                   EGL_OPENGL_ES2_BIT, EGL_RED_SIZE, 8, EGL_ALPHA_SIZE, 8, EGL_NONE };
    int iConfigs;

    if (!eglChooseConfig(g_eglContextStruct.eglDisplay, pi32ConfigAttribs, &g_eglContextStruct.eglConfig, 1, &iConfigs) || (iConfigs != 1))
    {
        printf("Error: eglChooseConfig() failed.\n");
    }

    g_eglContextStruct.eglSurface = eglCreateWindowSurface(
            g_eglContextStruct.eglDisplay, g_eglContextStruct.eglConfig,
            (EGLNativeWindowType) g_x11ContextStruct.x11Window, NULL);
    eglstatus = eglGetError();

    if (eglstatus != EGL_SUCCESS)
    {
        printf("Error: eglCreateWindowSurface() failed.\n"); // TODO: do not use printf directly here
    }

    g_eglContextStruct.eglContext = eglCreateContext(
            g_eglContextStruct.eglDisplay, g_eglContextStruct.eglConfig, NULL,
            contextAttribs);

    eglstatus = eglGetError();
    if (eglstatus != EGL_SUCCESS)
    {
        printf("Error: eglCreateContext() failed.\n");
    }

    eglMakeCurrent(g_eglContextStruct.eglDisplay,
            g_eglContextStruct.eglSurface, g_eglContextStruct.eglSurface,
            g_eglContextStruct.eglContext);
    eglSwapInterval(g_eglContextStruct.eglDisplay, 1);
    eglstatus = eglGetError();
    if (eglstatus != EGL_SUCCESS)
    {
        printf("Error: eglMakeCurrent() failed.\n");
    }

    // register surfaces to layermanager
    t_ilm_layer layerid = (t_ilm_layer)LAYER_EXAMPLE_GLES_APPLICATIONS;
    t_ilm_surface surfaceid = (t_ilm_surface)SURFACE_EXAMPLE_EGLX11_APPLICATION;

    printf("create a surface %lu\n", (t_ilm_nativehandle) g_x11ContextStruct.x11Window);
    ilm_surfaceCreate( (t_ilm_nativehandle) g_x11ContextStruct.x11Window, width, height,
            ILM_PIXELFORMAT_RGBA_8888, &surfaceid);

    printf("set surface dest region\n");
    ilm_surfaceSetDestinationRectangle(surfaceid, 0, 0, width, height);

    printf("set surface src region\n");
    ilm_surfaceSetSourceRectangle(surfaceid, 0, 0, width, height);

    printf("add surface to layer\n");
    ilm_layerAddSurface(layerid, surfaceid);

    printf("Set surface visible\n");
    ilm_surfaceSetVisibility(surfaceid, ILM_TRUE);

    printf("Set surface opacity\n");
    ilm_surfaceSetOpacity(surfaceid, 0.75f);

    printf("commit\n");
    return (ILM_SUCCESS == ilm_commitChanges()) ? ILM_TRUE : ILM_FALSE;
}

void destroyEglContext()
{
    if (g_eglContextStruct.eglDisplay != NULL)
    {
        eglMakeCurrent(g_eglContextStruct.eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglTerminate(g_eglContextStruct.eglDisplay);
    }
}

void destroyX11Context()
{
    if (g_x11ContextStruct.x11Window)
    {
        XDestroyWindow(g_x11ContextStruct.x11Display, g_x11ContextStruct.x11Window);
    }

    if (g_x11ContextStruct.x11Colormap)
    {
        XFreeColormap(g_x11ContextStruct.x11Display, g_x11ContextStruct.x11Colormap);
    }

    if (g_x11ContextStruct.x11Display)
    {
        XCloseDisplay(g_x11ContextStruct.x11Display);
    }

    if (g_x11ContextStruct.x11Visual)
    {
        free(g_x11ContextStruct.x11Visual);
    }
}

t_ilm_uint GetTickCount()
{
    struct timeval ts;
    gettimeofday(&ts, 0);
    return (t_ilm_uint) (ts.tv_sec * 1000 + (ts.tv_usec / 1000));
}

void swapBuffers()
{
    eglSwapBuffers(g_eglContextStruct.eglDisplay, g_eglContextStruct.eglSurface);
}
