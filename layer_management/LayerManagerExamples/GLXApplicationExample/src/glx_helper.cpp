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
#include "glx_helper.h"
#include "config.h"
#include "xevents.h"
#include "LayerScene.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

typedef struct t_glxContextStruct
{
    GLXContext glxContext;
} glxContextStruct;

typedef struct t_x11ContextStruct
{
    Window x11Window;
    Display* x11Display;
    long x11Screen;
    XVisualInfo* x11Visual;
    Colormap x11Colormap;
} X11ContextStruct;

static int attrListDbl[] =
{
    GLX_RGBA, GLX_DOUBLEBUFFER,
    GLX_RED_SIZE, 8,
    GLX_GREEN_SIZE, 8,
    GLX_BLUE_SIZE, 8,
    GLX_ALPHA_SIZE,8,
    GLX_DEPTH_SIZE, 24,
    None
};

static glxContextStruct g_glxContextStruct;
static X11ContextStruct g_x11ContextStruct;

t_ilm_bool createX11Context(t_ilm_int width, t_ilm_int height)
{
    t_ilm_bool result = ILM_TRUE;
    Window rootWindow;
    XSetWindowAttributes windowAttributes;
    unsigned int windowMask;
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
    g_x11ContextStruct.x11Screen
            = XDefaultScreen(g_x11ContextStruct.x11Display);

    // Get the root window parameters
    rootWindow = RootWindow(g_x11ContextStruct.x11Display, g_x11ContextStruct.x11Screen);
    /*TODO unused: int colorDepth  =*/ DefaultDepth(g_x11ContextStruct.x11Display, g_x11ContextStruct.x11Screen);

    // Alloc memory for the visual info

    g_x11ContextStruct.x11Visual = glXChooseVisual( g_x11ContextStruct.x11Display, 0, attrListDbl);
    if (!g_x11ContextStruct.x11Visual)
    {
        printf(" no doublebuffering available!\n");
    }

    g_x11ContextStruct.x11Screen = g_x11ContextStruct.x11Visual->screen;

    // Create the rendercontext color map
    g_x11ContextStruct.x11Colormap = XCreateColormap( g_x11ContextStruct.x11Display, rootWindow, g_x11ContextStruct.x11Visual->visual, AllocNone);
    windowAttributes.colormap = g_x11ContextStruct.x11Colormap;

    // Add to these for handling other events
    windowAttributes.event_mask = StructureNotifyMask | ExposureMask | ButtonPressMask | ButtonReleaseMask | KeyPressMask | KeyReleaseMask;
    windowAttributes.backing_store = Always;

    // Set the window mask attributes
    windowMask = CWBackPixel | CWBorderPixel | CWEventMask | CWColormap
            | CWBackingStore;

    // get the corrected window dimensions
    widthCorrected = width < XDisplayWidth(g_x11ContextStruct.x11Display, g_x11ContextStruct.x11Screen) ? width : XDisplayWidth(g_x11ContextStruct.x11Display, g_x11ContextStruct.x11Screen);
    heightCorrected = height < XDisplayHeight(g_x11ContextStruct.x11Display, g_x11ContextStruct.x11Screen) ? height : XDisplayHeight(g_x11ContextStruct.x11Display, g_x11ContextStruct.x11Screen);

    // Creates the X11 window
    g_x11ContextStruct.x11Window = XCreateWindow(
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

t_ilm_bool createGLXContext(t_ilm_int width, t_ilm_int height)
{
    int glxMajor;
    int glxMinor;

    g_glxContextStruct.glxContext = glXCreateContext( g_x11ContextStruct.x11Display, g_x11ContextStruct.x11Visual, 0, GL_TRUE);
    glXMakeCurrent(g_x11ContextStruct.x11Display, g_x11ContextStruct.x11Window, g_glxContextStruct.glxContext);

    // do egl stuff
    glXQueryVersion(g_x11ContextStruct.x11Display, &glxMajor, &glxMinor);
    printf("GLX-Version %d.%d\n", glxMajor, glxMinor);

    if (glXIsDirect(g_x11ContextStruct.x11Display, g_glxContextStruct.glxContext))
    {
        printf("DRI enabled\n");
    }
    else
    {
        printf("no DRI available\n");
    }

    t_ilm_layer layerid = (t_ilm_layer) LAYER_EXAMPLE_X_APPLICATIONS; // TODO: remove all C stylec asts in C++ code
    t_ilm_surface surfaceid = (t_ilm_surface) SURFACE_EXAMPLE_GLXX11_APPLICATION;

    printf("create a surface %lu\n", (t_ilm_nativehandle) (g_x11ContextStruct.x11Window));

    ilm_surfaceCreate((t_ilm_nativehandle) g_x11ContextStruct.x11Window, width, height, ILM_PIXELFORMAT_RGBA_8888, &surfaceid);

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

    xeventInitialiaze(g_x11ContextStruct.x11Display, surfaceid, 0, 0, width, height);

    printf("commit\n");
    // commit will indicate error, if any command failed
    ilmErrorTypes error = ilm_commitChanges();

    return (error != ILM_FAILED) ? ILM_TRUE : ILM_FALSE;
}

void destroyGLXContext()
{
    if (g_glxContextStruct.glxContext)
    {
        glXMakeCurrent(g_x11ContextStruct.x11Display, g_x11ContextStruct.x11Window, g_glxContextStruct.glxContext);
        glXDestroyContext(g_x11ContextStruct.x11Display, g_glxContextStruct.glxContext);
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
    glXSwapBuffers(g_x11ContextStruct.x11Display, g_x11ContextStruct.x11Window);
}
