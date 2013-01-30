/***************************************************************************
 *
 * Copyright 2010,2011 BMW Car IT GmbH
 * Copyright (C) 2012 DENSO CORPORATION and Robert Bosch Car Multimedia Gmbh
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
#include "wayland-client.h"
#include "wayland-client-protocol.h"
#include "ilm_client.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>
#include <getopt.h>

#include "serverinfo-client-protocol.h"
#include "serverinfo-protocol.c"

#include "bmpaccessor.h"

/*****************************************************************************
 *  local defines
 ****************************************************************************/
#define LAYER_CHROMAKEY   3500
#define SURFACE_CHROMAKEY 6789

#define CHECK_ILM_ERROR(S)      \
    if ((S) != ILM_SUCCESS){    \
        printf("%s (%d): ilm command failed.\n", __FILE__, __LINE__); \
        break;                  \
    }                           \

/*****************************************************************************
 *  structure
 ****************************************************************************/
typedef struct t_wlContextStruct {
    struct wl_display*    wlDisplay;
    struct wl_registry*   wlRegistry;
    struct wl_compositor* wlCompositor;
    struct wl_surface*    wlSurface;
    struct wl_shm*        wlShm;
    uint32_t              formats;
    struct serverinfo*    wlExtServerinfo;
    uint32_t              connect_id;
    ctx_bmpaccessor*      ctx_bmp;
    void*                 data;
    struct wl_buffer*     wlBuffer;
} WLContextStruct;

/*****************************************************************************
 *  globals
 ****************************************************************************/
WLContextStruct g_wlContextStruct;
int gRun = 0;

static void serverinfoListener(void *data, struct serverinfo *pServerinfo, uint32_t client_handle)
{
    WLContextStruct* p_wlCtx = (WLContextStruct*)data;
    p_wlCtx->connect_id = client_handle;
}

struct serverinfo_listener serverinfo_listener_list = {
    serverinfoListener
};

static void shm_format(void* data, struct wl_shm* pWlShm, uint32_t format)
{
    WLContextStruct* pDsp = data;
    pDsp->formats |= (1 << format);
}

struct wl_shm_listener shm_listenter = {
    shm_format
};

static void registry_handle_global(void* data, struct wl_registry* registry, uint32_t name, const char* interface, uint32_t version)
{
    WLContextStruct* p_wlCtx = (WLContextStruct*)data;
    int ans_strcmp = 0;

    do
    {
        ans_strcmp = strcmp(interface, "wl_compositor");
        if (0 == ans_strcmp)
        {
            p_wlCtx->wlCompositor = (struct wl_compositor*)wl_registry_bind(registry, name, &wl_compositor_interface, 1);
            break;
        }

        ans_strcmp = strcmp(interface, "wl_shm");
        if (0 == ans_strcmp)
        {
            p_wlCtx->wlShm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
            wl_shm_add_listener(p_wlCtx->wlShm, &shm_listenter, p_wlCtx);
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

/*****************************************************************************
 *  local functions
 ****************************************************************************/
static void createShmBuffer()
{
    struct wl_shm_pool *pool;

    char filename[] = "/tmp/wayland-shm-XXXXXX";
    int fd = -1;
    int size = 0;

    fd = mkstemp(filename);
    if (fd < 0){
        fprintf(stderr, "open %s failed: %m\n", filename);
        return;
    }
    size = g_wlContextStruct.ctx_bmp->stride * g_wlContextStruct.ctx_bmp->height;
    if (ftruncate(fd, size) < 0){
        fprintf(stderr, "ftruncate failed: %m\n");
        close(fd);
        return;
    }

    g_wlContextStruct.data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (MAP_FAILED == g_wlContextStruct.data)
    {
        fprintf(stderr, "mmap failed: %m\n");
        close(fd);
        return;
    }

    pool = wl_shm_create_pool(g_wlContextStruct.wlShm, fd, size);
    g_wlContextStruct.wlBuffer = wl_shm_pool_create_buffer(pool, 0, g_wlContextStruct.ctx_bmp->width,
                                                g_wlContextStruct.ctx_bmp->height,
                                                g_wlContextStruct.ctx_bmp->stride,
                                                WL_SHM_FORMAT_XRGB8888);

    if (NULL == g_wlContextStruct.wlBuffer)
    {
        fprintf(stderr, "wl_shm_create_buffer failed: %m\n");
        close(fd);
        return;
    }
    wl_surface_attach(g_wlContextStruct.wlSurface, g_wlContextStruct.wlBuffer, 0, 0);
    wl_shm_pool_destroy(pool);
    close(fd);

    return;
}

static void destroyWLContext()
{
    if (g_wlContextStruct.wlSurface)
    {
        wl_surface_destroy(g_wlContextStruct.wlSurface);
    }
    if (g_wlContextStruct.wlCompositor)
    {
        wl_compositor_destroy(g_wlContextStruct.wlCompositor);
    }
}

static int createWLContext()
{
    t_ilm_bool result = ILM_TRUE;

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
        printf("Error: wl_compositor_create_surface failed.\n");
        destroyWLContext();
    }

    createShmBuffer();

    return result;
}

static void frame_listener_func(void *data, struct wl_callback *callback, uint32_t time)
{
    if (callback)
    {
        wl_callback_destroy(callback);
    }
}

static const struct wl_callback_listener frame_listener = {
    frame_listener_func
};

static void drawImage(void)
{
    memcpy(g_wlContextStruct.data,
           g_wlContextStruct.ctx_bmp->data,
           g_wlContextStruct.ctx_bmp->stride * g_wlContextStruct.ctx_bmp->height);

    wl_surface_attach(g_wlContextStruct.wlSurface,
                      g_wlContextStruct.wlBuffer,
                      0, 0);
    wl_surface_damage(g_wlContextStruct.wlSurface,
                      0, 0,
                      g_wlContextStruct.ctx_bmp->width,
                      g_wlContextStruct.ctx_bmp->height);
    struct wl_callback* callback = wl_surface_frame(g_wlContextStruct.wlSurface);
    wl_callback_add_listener(callback, &frame_listener, NULL);
    wl_surface_commit(g_wlContextStruct.wlSurface);
    //wl_display_flush(g_wlContextStruct.wlDisplay);
    wl_display_roundtrip(g_wlContextStruct.wlDisplay);
}

static ilmErrorTypes createILMAttribute(t_ilm_layer *pLayerId, t_ilm_surface *pSurfaceId, unsigned int colorkey[])
{
    ilmErrorTypes error = ILM_FAILED;
    t_ilm_layer   layerid   = *pLayerId;
    t_ilm_surface surfaceid = *pSurfaceId;

    do {
        printf("Creating Layer... ");
        error = ilm_layerCreateWithDimension(&layerid,
                                             g_wlContextStruct.ctx_bmp->width,
                                             g_wlContextStruct.ctx_bmp->height);
        CHECK_ILM_ERROR(error);

        printf("Setting Layer destination rectangle(0, 0, %d, %d)... \n",
            g_wlContextStruct.ctx_bmp->width,
            g_wlContextStruct.ctx_bmp->height);
        error = ilm_layerSetDestinationRectangle(layerid,
                                                 0, 0,
                                                 g_wlContextStruct.ctx_bmp->width,
                                                 g_wlContextStruct.ctx_bmp->height);
        CHECK_ILM_ERROR(error);

        printf("Setting Layer source rectangle(0, 0, %d, %d)... \n",
            g_wlContextStruct.ctx_bmp->width,
            g_wlContextStruct.ctx_bmp->height);
        error = ilm_layerSetSourceRectangle(layerid,
                                            0, 0,
                                            g_wlContextStruct.ctx_bmp->width,
                                            g_wlContextStruct.ctx_bmp->height);
        CHECK_ILM_ERROR(error);

        printf("Setting Layer visibility(%d)... \n", 1);
        error = ilm_layerSetVisibility(layerid, ILM_TRUE);
        CHECK_ILM_ERROR(error);

        error = ilm_layerSetOpacity(layerid, 1.0f);
        CHECK_ILM_ERROR(error);

        struct wl_object* p_obj = (struct wl_object*)g_wlContextStruct.wlSurface;
        t_ilm_nativehandle native_ilm_handle = (g_wlContextStruct.connect_id << 16) | (uint32_t)p_obj->id;
        error = ilm_surfaceCreate(native_ilm_handle,
                                  g_wlContextStruct.ctx_bmp->width,
                                  g_wlContextStruct.ctx_bmp->height,
                                  ILM_PIXELFORMAT_RGBA_8888,
                                  &surfaceid);
        CHECK_ILM_ERROR(error);

        printf("set surface dest region\n");
        error = ilm_surfaceSetDestinationRectangle(surfaceid,
                                                   0, 0,
                                                   g_wlContextStruct.ctx_bmp->width,
                                                   g_wlContextStruct.ctx_bmp->height);
        CHECK_ILM_ERROR(error);


        printf("set surface src region\n");
        error = ilm_surfaceSetSourceRectangle(surfaceid,
                                              0, 0,
                                              g_wlContextStruct.ctx_bmp->width,
                                              g_wlContextStruct.ctx_bmp->height);
        CHECK_ILM_ERROR(error);


        printf("add surface to layer\n");
        error = ilm_layerAddSurface(layerid, surfaceid);
        CHECK_ILM_ERROR(error);

        printf("Set surface visible\n");
        error = ilm_surfaceSetVisibility(surfaceid, ILM_TRUE);
        CHECK_ILM_ERROR(error);

        printf("Set surface opacity\n");
        error = ilm_surfaceSetOpacity(surfaceid, 0.7f);
        CHECK_ILM_ERROR(error);

        printf("Set surface chromakey\n");
        //unsigned int col[3] = {255, 242, 0};
        error = ilm_surfaceSetChromaKey(surfaceid, colorkey);
        CHECK_ILM_ERROR(error);

        printf("commit\n");
        error = ilm_commitChanges();
        CHECK_ILM_ERROR(error);

    } while (0);

    *pLayerId   = layerid;
    *pSurfaceId = surfaceid;

    return error;
}

static void sigFunc()
{
    printf("Catch signal\n");
    gRun = 0;
}

static void print_usage()
{
    printf("Usage: WLChromakeyExample -f [BITMAP FILE] -c [R,G,B]\n");
    printf("Chromakey for surface example (wayland backend).\n");
    printf("\n");
    printf("Option:\n");
    printf("-f, --file=FILE            bitmap file of uncompressed RGB888(24BPP) format\n");
    printf("-c, --color=COLOR(R,G,B)   color value which defines the transparency value\n");
    printf("                           default values are 255,255,255\n");
}

/*****************************************************************************
 *  exported functions
 ****************************************************************************/

int main(int argc, char *argv[])
{
    ilmErrorTypes result = ILM_FAILED;
    t_ilm_layer layerid = (t_ilm_layer)LAYER_CHROMAKEY;
    t_ilm_surface surfaceid = (t_ilm_surface)SURFACE_CHROMAKEY;
    char* bitmapFile = NULL;
    int o;
    static const char opts[] = "f:c:h";
    static const struct option longopts[] = {
        { "file",  1, NULL, 'f' },
        { "color", 1, NULL, 'c' },
        { "help",  1, NULL, 'h' },
        { NULL, }
    };
    unsigned int rgb[] = {255, 255, 255}; /* Default color key is 'White' */

    while (o = getopt_long(argc, argv, opts, longopts, &o), o > 0) {
        switch (o) {
        case 'f':
            bitmapFile = optarg;
            break;
        case 'c':
            sscanf(optarg, "%d,%d,%d", &rgb[0], &rgb[1], &rgb[2]);
            break;
        case 'h':
            print_usage();
            return 0;
        }
    }

    if (bitmapFile == NULL) {
        print_usage();
        return -1;
    }

    printf("[INFO] bitmap file = %s\n", bitmapFile);
    printf("[INFO] color key   = R(%d),G(%d),B(%d)\n", rgb[0], rgb[1], rgb[2]);

    /* signal handling */
    signal(SIGINT,  sigFunc);
    signal(SIGKILL, sigFunc);

    gRun = 1;
    memset(&g_wlContextStruct, 0x00, sizeof(g_wlContextStruct));

    g_wlContextStruct.ctx_bmp = bmpaccessor_open(bitmapFile);
    if (NULL == g_wlContextStruct.ctx_bmp)
    {
        printf("Failed to create bmp accessor\n");
        return -1;
    }

    do {
        createWLContext();

        result = ilm_init();
        if (ILM_SUCCESS == result)
        {
            printf("ilm_init success\n");
        }
        else
        {
            printf("ilm_init failed\n");
            break;
        }
        result = createILMAttribute(&layerid, &surfaceid, rgb);
        if (result != ILM_SUCCESS)
        {
            break;
        }

        t_ilm_layer layer[] = {1000, 2000, 3000, LAYER_CHROMAKEY};
        ilm_displaySetRenderOrder(0, &layer[0], 4);

        ilm_commitChanges();

        while(gRun) {
            drawImage();
        }

        ilm_surfaceRemove(surfaceid);
        ilm_layerRemove(layerid);
    } while(0);

    ilm_destroy();
    destroyWLContext();

    bmpaccessor_close(g_wlContextStruct.ctx_bmp);

    return 0;
}
