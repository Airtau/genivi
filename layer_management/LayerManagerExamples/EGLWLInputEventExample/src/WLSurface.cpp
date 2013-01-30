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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "WLSurface.h"

WLSurface::WLSurface(WLContext* wlContext)
: m_wlContext(wlContext)
, m_wlSurface(NULL)
, m_width(0)
, m_height(0)
, m_ilmLayerId(0)
, m_ilmSurfaceId(0)
{
    assert(wlContext);
}

WLSurface::~WLSurface()
{
    if (m_ilmSurfaceId > 0)
        ilm_surfaceRemove(m_ilmSurfaceId);

    if (m_wlSurface)
        wl_surface_destroy(m_wlSurface);
}

bool
WLSurface::CreateSurface(const int width, const int height)
{
    m_width  = width;
    m_height = height;

    m_wlSurface = (struct wl_surface*)
        wl_compositor_create_surface(m_wlContext->GetWLCompositor());
    if (!m_wlSurface){
        return false;
    }

    return CreatePlatformSurface();
}

bool
WLSurface::CreatePlatformSurface()
{
    return true;
}

bool
WLSurface::CreateIlmSurface(t_ilm_layer* layerId,
                            t_ilm_surface* surfaceId,
                            t_ilm_int width,
                            t_ilm_int height)
{
    ilmErrorTypes rtnv;
    struct wl_object* wlObj = (struct wl_object*)m_wlSurface;
    uint32_t nativeHandle = m_wlContext->GetConnectionId() << 16 | (uint32_t)wlObj->id;

    // Layer is already exist
    rtnv = ilm_layerSetDestinationRectangle(*layerId, 0, 0, 800, 480);
    rtnv = ilm_layerSetSourceRectangle(*layerId, 0, 0, 800, 480);

    // Creates surfce
    rtnv = ilm_surfaceCreate((t_ilm_nativehandle)nativeHandle,
                             width, height,
                             ILM_PIXELFORMAT_RGBA_8888, surfaceId);
    if (rtnv != ILM_SUCCESS){
        return false;
    }

    rtnv = ilm_surfaceSetDestinationRectangle(*surfaceId, 50, 50, width, height);
    rtnv = ilm_surfaceSetSourceRectangle(*surfaceId, 0, 0, width, height);
    rtnv = ilm_surfaceSetVisibility(*surfaceId, ILM_TRUE);
    rtnv = ilm_surfaceSetOpacity(*surfaceId, 1.0f);

    rtnv = ilm_layerAddSurface(*layerId, *surfaceId);
    rtnv = ilm_UpdateInputEventAcceptanceOn(*surfaceId,
                                            ILM_INPUT_DEVICE_POINTER | ILM_INPUT_DEVICE_TOUCH,
                                            ILM_TRUE);
    rtnv = ilm_SetKeyboardFocusOn(*surfaceId);

    rtnv = ilm_commitChanges();

    m_ilmLayerId   = *layerId;
    m_ilmSurfaceId = *surfaceId;

    return true;
}

void
WLSurface::DestroyIlmSurface()
{
    if (m_ilmSurfaceId > 0)
        ilm_surfaceRemove(m_ilmSurfaceId);
}
