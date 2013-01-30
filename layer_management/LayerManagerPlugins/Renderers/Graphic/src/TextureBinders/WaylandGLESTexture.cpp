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
****************************************************************************/

#include "TextureBinders/WaylandGLESTexture.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2ext.h>
#include "PlatformSurfaces/EglWaylandPlatformSurface.h"
#include "Log.h"
#include "wayland-server.h"
#include "config.h"

WaylandGLESTexture::WaylandGLESTexture(EGLDisplay eglDisplay, struct wl_display* wlDisplay)
: m_eglDisplay(eglDisplay)
, m_wlDisplay(wlDisplay)
{
    // pseudo require EGL to have been initialised
    // we dont really need the EGL handle as such

    m_pfGLEglImageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)eglGetProcAddress("glEGLImageTargetTexture2DOES");
    m_pfEglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC)eglGetProcAddress("eglCreateImageKHR");
    m_pfEglDestroyImageKHR = (PFNEGLDESTROYIMAGEKHRPROC)eglGetProcAddress("eglDestroyImageKHR");

    if (!m_pfEglCreateImageKHR || !m_pfEglDestroyImageKHR || !m_pfGLEglImageTargetTexture2DOES)
    {
        LOG_ERROR("WaylandGLESTexture", "Query EGL Extensions failed");
    }
}

WaylandGLESTexture::~WaylandGLESTexture()
{
}

bool WaylandGLESTexture::bindSurfaceTexture(Surface* surface)
{
    LOG_DEBUG("WaylandGLESTexture", "bindSurfaceTexture IN");
    EglWaylandPlatformSurface* nativeSurface = (EglWaylandPlatformSurface*)surface->platform;
    if (nativeSurface && nativeSurface->isReadyForRendering())
    {
        struct wl_buffer* buffer = (struct wl_buffer*)surface->getNativeContent();
        if (wl_buffer_is_shm(buffer))
        {
            /* Wayland SHM buffer */
            unsigned int* pBuf = (unsigned int*)wl_shm_buffer_get_data(buffer);
            if( (NULL != buffer) && (NULL != pBuf) )
            {
                LOG_DEBUG("WaylandGLESTexture", "SHM buffer address:"<<pBuf);
                unsigned int* pTmp = (unsigned int*)pBuf;
                unsigned int col = 0;
                int cnt = 0;
                unsigned int* pCnvImg = (unsigned int*)malloc(sizeof(unsigned int) * surface->OriginalSourceWidth * surface->OriginalSourceHeight);
                unsigned int* pCnvImgTmp = pCnvImg;
                for (cnt = 0; cnt < surface->OriginalSourceWidth * surface->OriginalSourceHeight; cnt++, pTmp++, pCnvImgTmp++)
                {
                    col = (unsigned int)*pTmp;
                    // TODO:endian
                    *pCnvImgTmp = (col&0xFF00FF00) | ((col&0x00FF0000)>>16) | ((col&0x000000FF)<<16);
                }

                glBindTexture(GL_TEXTURE_2D, nativeSurface->texture);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->OriginalSourceWidth, surface->OriginalSourceHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, pCnvImg);
                free(pCnvImg);
            }
            return true;

        }
        else
        {
            /* Wayland Not SHM buffer */
            if (nativeSurface->eglImage)
            {
                LOG_DEBUG("WaylandGLESTexture", "nativeSurface->eglImage:"<<nativeSurface->eglImage);
                glBindTexture(GL_TEXTURE_2D, nativeSurface->texture);
                return true;
            }
        }

        LOG_DEBUG("WaylandGLESTexture", "bindSurfaceTexture OUT");
        return true;
    }
    LOG_DEBUG("WaylandGLESTexture", "bindSurfaceTexture ERROR OUT");
    return false;
}

bool WaylandGLESTexture::unbindSurfaceTexture(Surface* surface)
{
    LOG_DEBUG("WaylandGLESTexture", "unbindSurfaceTexture:surface" << surface);
    return true;
}

void WaylandGLESTexture::createClientBuffer(Surface* surface)
{
    LOG_DEBUG("WaylandGLESTexture", "creating client buffer with native display: " << m_wlDisplay << " for native handle: " << surface->getNativeContent());
    EglWaylandPlatformSurface* nativeSurface = (EglWaylandPlatformSurface*)surface->platform;

    if (NULL!=nativeSurface)
    {
        struct wl_buffer* buffer = (struct wl_buffer*)surface->getNativeContent();
        if (wl_buffer_is_shm(buffer))
        {
            if (nativeSurface->texture)
	    {
                glDeleteTextures(1,&nativeSurface->texture);
                nativeSurface->texture = 0;
            }
            glGenTextures(1,&nativeSurface->texture);
        }
        else
        {
            EGLImageKHR eglImage = 0;
            LOG_DEBUG("WaylandGLESTexture", "creating EGL Image from client buffer");
            if (nativeSurface->eglImage)
            {
                m_pfEglDestroyImageKHR(m_eglDisplay, nativeSurface->eglImage);
                glDeleteTextures(1,&nativeSurface->texture);
                nativeSurface->eglImage = 0;
                nativeSurface->texture = 0;
            }
            eglImage = m_pfEglCreateImageKHR(m_eglDisplay,
                                         EGL_NO_CONTEXT,
                                         EGL_WAYLAND_BUFFER_WL,
                                         (EGLClientBuffer)buffer,
                                         NULL);
            if (!eglImage)
            {
                LOG_DEBUG("WaylandGLESTexture", "could not allocate EGL Image for window");
            }
            else
            {
                nativeSurface->eglImage = eglImage;
                glGenTextures(1,&nativeSurface->texture);
                glBindTexture(GL_TEXTURE_2D, nativeSurface->texture);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                m_pfGLEglImageTargetTexture2DOES(GL_TEXTURE_2D, nativeSurface->eglImage);
           }
        }
    }
}

PlatformSurface* WaylandGLESTexture::createPlatformSurface(Surface* surface)
{
    return new EglWaylandPlatformSurface(surface);
}

void WaylandGLESTexture::destroyClientBuffer(Surface* surface)
{
    EglWaylandPlatformSurface* nativeSurface = (EglWaylandPlatformSurface*)surface->platform;

    struct wl_buffer* buffer = (struct wl_buffer*)surface->getNativeContent();
    if (wl_buffer_is_shm(buffer))
    {
        if (nativeSurface && nativeSurface->texture)
        {
            glDeleteTextures(1,&nativeSurface->texture);
            nativeSurface->texture = 0;
        }
    }
    else
    {
        if (nativeSurface && nativeSurface->eglImage)
        {
            m_pfEglDestroyImageKHR(m_eglDisplay, nativeSurface->eglImage);
            glDeleteTextures(1,&nativeSurface->texture);
            nativeSurface->eglImage = 0;
            nativeSurface->texture = 0;
        }
    }
}
