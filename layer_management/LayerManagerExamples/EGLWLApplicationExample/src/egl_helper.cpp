/**************************************************************************
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
 ****************************************************************************/
#include "egl_helper.h"
#include "LayerScene.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#include <stdint.h>
#include <signal.h>
#include <sys/stat.h>
#include <linux/fb.h>

#include "WaylandServerinfoClientProtocol.h"

/* #define USE_PIXMAP */
typedef struct t_eglContextStruct
{
    EGLNativeDisplayType nativeDisplay;
    EGLNativeWindowType  nativeWindow;
    EGLDisplay eglDisplay;
    EGLConfig eglConfig;
    EGLSurface eglSurface;
    EGLContext eglContext;
} EglContextStruct;

typedef struct t_wlContextStruct
{
    struct wl_display* wlDisplay;
    struct wl_registry* wlRegistry;
    struct wl_compositor* wlCompositor;

    struct wl_surface* wlSurface;
    struct wl_egl_window* wlNativeWindow;
    int width;
    int height;

    struct serverinfo* wlExtServerinfo;
    uint32_t connect_id;
} WLContextStruct;

static EglContextStruct g_eglContextStruct;
static WLContextStruct g_wlContextStruct;

static void serverinfoListener(void *data, struct serverinfo *pServerinfo, uint32_t client_handle)
{
    pServerinfo = pServerinfo; // TODO:to avoid warning
    WLContextStruct* p_wlCtx = (WLContextStruct*)data;
    p_wlCtx->connect_id = client_handle;
    fprintf( stderr, "notified wayland connection : id=%d\n", p_wlCtx->connect_id);
}

struct serverinfo_listener serverinfo_listener_list = {
    serverinfoListener
};

static void registry_handle_global(void* data, struct wl_registry* registry, uint32_t name, const char* interface, uint32_t version)
{
    version = version; // TODO: to avoid warning
    WLContextStruct* p_wlCtx = (WLContextStruct*)data;
    int ans_strcmp = 0;

    do
    {
        ans_strcmp = strcmp(interface, "wl_compositor");
        if (0 == ans_strcmp)
        {
            p_wlCtx->wlCompositor = (wl_compositor*)wl_registry_bind(registry, name, &wl_compositor_interface, 1);
            break;
        }

        ans_strcmp = strcmp(interface, "serverinfo");
        if (0 == ans_strcmp)
        {
            p_wlCtx->wlExtServerinfo = (struct serverinfo*)wl_registry_bind(registry, name, &serverinfo_interface, 1);
            serverinfo_add_listener(p_wlCtx->wlExtServerinfo, &serverinfo_listener_list, data);
            serverinfo_get_connection_id(p_wlCtx->wlExtServerinfo);
        }
    } while(0);
}

static const struct wl_registry_listener registry_listener = {
    registry_handle_global,
    NULL
};

t_ilm_bool createWLContext(t_ilm_int width, t_ilm_int height)
{
    t_ilm_bool result = ILM_TRUE;

    memset(&g_wlContextStruct, 0, sizeof(g_wlContextStruct));

    g_wlContextStruct.width = width;
    g_wlContextStruct.height = height;
    g_wlContextStruct.wlDisplay = wl_display_connect(NULL);
    if (NULL == g_wlContextStruct.wlDisplay)
    {
        printf("Error: wl_display_connect() failed.\n");
    }

    g_wlContextStruct.wlRegistry = wl_display_get_registry(g_wlContextStruct.wlDisplay);
    wl_registry_add_listener(g_wlContextStruct.wlRegistry, &registry_listener, &g_wlContextStruct);
    wl_display_dispatch(g_wlContextStruct.wlDisplay);
    wl_display_roundtrip(g_wlContextStruct.wlDisplay);

    g_wlContextStruct.wlSurface = wl_compositor_create_surface(g_wlContextStruct.wlCompositor);
    if (NULL == g_wlContextStruct.wlSurface)
    {
        printf("Error: wl_compositor_create_surface() failed.\n");
        destroyWLContext();
    }

    g_wlContextStruct.wlNativeWindow = wl_egl_window_create(g_wlContextStruct.wlSurface, width, height);
    if (NULL == g_wlContextStruct.wlNativeWindow)
    {
        printf("Error: wl_egl_window_create() failed.\n");
        destroyWLContext();
    }

    return result;
}

EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };

t_ilm_bool createEGLContext(t_ilm_int width, t_ilm_int height)
{
    t_ilm_bool result = ILM_TRUE;
    g_eglContextStruct.eglDisplay = NULL;
    g_eglContextStruct.eglSurface = NULL;
    g_eglContextStruct.eglContext = NULL;
    ilmErrorTypes error = ILM_FAILED;
    uint32_t native_ilm_handle = 0;
    EGLint eglstatus = EGL_SUCCESS;

    g_eglContextStruct.eglDisplay = eglGetDisplay(g_wlContextStruct.wlDisplay);
    eglstatus = eglGetError();
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
    EGLint pi32ConfigAttribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
        EGL_RED_SIZE,   8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE,  8,
        EGL_ALPHA_SIZE, 8,
        EGL_NONE
    };
    int iConfigs;

    if (!eglChooseConfig(g_eglContextStruct.eglDisplay, pi32ConfigAttribs, &g_eglContextStruct.eglConfig, 1, &iConfigs) || (iConfigs != 1))
    {
        printf("Error: eglChooseConfig() failed.\n");
    }
    eglstatus = eglGetError();
    if (eglstatus != EGL_SUCCESS)
    {
        printf("Error: eglChooseConfig() failed. EGLStatus:0x%x\n", (unsigned int)eglstatus); // TODO: do not use printf directly here
    }

    g_eglContextStruct.eglSurface = eglCreateWindowSurface(
            g_eglContextStruct.eglDisplay, g_eglContextStruct.eglConfig,
            g_wlContextStruct.wlNativeWindow, NULL);
    eglstatus = eglGetError();

    if (eglstatus != EGL_SUCCESS)
    {
        printf("Error: eglCreateWindowSurface() failed. EGLStatus:0x%u\n", (unsigned int)eglstatus); // TODO: do not use printf directly here
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
    t_ilm_layer layerid = (t_ilm_layer)LAYER_EXAMPLE_VIDEO_APPLICATIONS;
    t_ilm_surface surfaceid = (t_ilm_surface)SURFACE_EXAMPLE_EGLX11_APPLICATION;

    // TODO: if (error == ILM_FAILED) return ILM_FALSE;

    // TODO: auto generate surface id
    struct wl_object* p_obj = (struct wl_object*)g_wlContextStruct.wlSurface;
    native_ilm_handle = (g_wlContextStruct.connect_id << 16) | (uint32_t)p_obj->id;
    error = ilm_surfaceCreate( (t_ilm_nativehandle) native_ilm_handle, width, height,
            ILM_PIXELFORMAT_RGBA_8888, &surfaceid);

    if (error == ILM_FAILED)
    {
        printf("ERROR: create a surface\n");
    }
    // TODO: if (error == ILM_FAILED) return ILM_FALSE;

    printf("set surface dest region\n");
    error = ilm_surfaceSetDestinationRectangle(surfaceid, 0, 0, width, height);

    // TODO: if (error == ILM_FAILED) return ILM_FALSE;

    printf("set surface src region\n");
    //error = ilm_surfaceSetSourceRectangle(surfaceid, 0, 0, width, height);
    error = ilm_surfaceSetSourceRectangle(surfaceid, 0, 0, width, height);

    // TODO: if (error == ILM_FAILED) return ILM_FALSE;

    printf("add surface to layer\n");
    error = ilm_layerAddSurface(layerid, surfaceid);
    printf("Set surface visible\n");
    error = ilm_surfaceSetVisibility(surfaceid, ILM_TRUE);
    printf("Set surface opacity\n");
    error = ilm_surfaceSetOpacity(surfaceid, 0.75f);

    //if (error == ILM_FAILED) return ILM_FALSE;

    printf("commit\n");
    error = ilm_commitChanges();

    //if (error == ILM_FAILED) return ILM_FALSE;

    return result;
}

void destroyEglContext()
{
    if (g_eglContextStruct.eglDisplay != NULL)
    {
        eglMakeCurrent(g_eglContextStruct.eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglTerminate(g_eglContextStruct.eglDisplay);
    }
}

void destroyWLContext()
{
    if (g_wlContextStruct.wlNativeWindow)
    {
        wl_egl_window_destroy(g_wlContextStruct.wlNativeWindow);
    }
    if (g_wlContextStruct.wlSurface)
    {
        wl_surface_destroy(g_wlContextStruct.wlSurface);
    }
    if (g_wlContextStruct.wlCompositor)
    {
        wl_compositor_destroy(g_wlContextStruct.wlCompositor);
    }
}

t_ilm_uint GetTickCount()
{
    struct timeval ts;
    gettimeofday(&ts, 0);
    return (t_ilm_uint) (ts.tv_sec * 1000 + (ts.tv_usec / 1000));
}

static void frame_listener_func(void *data, struct wl_callback *callback, uint32_t time)
{
    data = data; // TODO:to avoid warning
    time = time; // TODO:to avoid warining
    if (callback)
    {
        wl_callback_destroy(callback);
    }
}

static const struct wl_callback_listener frame_listener = {
    frame_listener_func	
};

void swapBuffers()
{
    eglSwapBuffers(g_eglContextStruct.eglDisplay, g_eglContextStruct.eglSurface);

    struct wl_callback* callback = wl_surface_frame(g_wlContextStruct.wlSurface);
    wl_callback_add_listener(callback, &frame_listener, NULL);
    wl_surface_commit(g_wlContextStruct.wlSurface);
    wl_display_flush(g_wlContextStruct.wlDisplay);
}
