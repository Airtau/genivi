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

#ifndef _BEAGLEEGLIMAGE_H_
#define _BEAGLEEGLIMAGE_H_

#include "TextureBinders/ITextureBinder.h"
#include <EGL/egl.h>
#include "EGL/eglext.h"
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <cmem.h>
#include "PlatformSurfaces/BeaglePlatformSurface.h"

class BeagleEglImage: public ITextureBinder
{
public:
    BeagleEglImage(EGLDisplay eglDisplay)
    : m_eglDisplay(eglDisplay)
    {
        LOG_INFO("BeagleEglImage", "Query EGL Extensions ...");
        CMEM_init();
        m_pfGLEglImageTargetTexture2DOES = (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC) eglGetProcAddress("glEGLImageTargetTexture2DOES");
        m_pfEglCreateImageKHR = (PFNEGLCREATEIMAGEKHRPROC) eglGetProcAddress("eglCreateImageKHR");
        m_pfEglDestroyImageKHR = (PFNEGLDESTROYIMAGEKHRPROC) eglGetProcAddress("eglDestroyImageKHR");

        if (!m_pfEglCreateImageKHR || !m_pfEglDestroyImageKHR || !m_pfGLEglImageTargetTexture2DOES)
        {
            LOG_ERROR("BeagleEglImage", "Query EGL Extensions failed");
        }
    }

    void bindSurfaceTexture(Surface*s)
    {
        //        LOG_INFO("BeagleEglImage", "bindSurfaceTexture");
        BeaglePlatformSurface* nativeSurface = (BeaglePlatformSurface*) s->platform;
        glBindTexture(GL_TEXTURE_2D, nativeSurface->texture);
        if (nativeSurface->eglImage != NULL)
        {
            //            LOG_INFO("BeagleEglImage", "bindSurfaceTexture");
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            eglWaitNative(EGL_CORE_NATIVE_ENGINE);
            m_pfGLEglImageTargetTexture2DOES(GL_TEXTURE_2D, nativeSurface->eglImage);
        }
    }

    void unbindSurfaceTexture(Surface*s)
    {

    }

    PlatformSurface* createPlatformSurface(Surface* s)
    {
        return new BeaglePlatformSurface(s);
    }

    void createClientBuffer(Surface* surface)
    {
        LOG_DEBUG("BeagleEglImage", "createClientBuffer");
        BeaglePlatformSurface* nativeSurface = (BeaglePlatformSurface*) surface->platform;
        NATIVE_PIXMAP_STRUCT* pNativePixmap = (NATIVE_PIXMAP_STRUCT*) malloc(sizeof(NATIVE_PIXMAP_STRUCT));
        pNativePixmap->ePixelFormat = 2;
        pNativePixmap->eRotation = 0;
        pNativePixmap->lWidth = 256;//surface->OriginalSourceWidth;//480;
        pNativePixmap->lHeight = 256;//surface->OriginalSourceHeight;//640;
        pNativePixmap->lStride = pNativePixmap->lWidth * 32 / 8; //bitwidth/8
        pNativePixmap->lSizeInBytes = pNativePixmap->lHeight * pNativePixmap->lStride;
        pNativePixmap->lAddress = (long) CMEM_registerAlloc(surface->nativeHandle);
        if (!pNativePixmap->lAddress)
        {
            LOG_ERROR("BeagleEglImage", "CMEM_alloc returned NULL\n");
            exit(-1);
        }

        //Get the physical page corresponding to the above cmem buffer
        pNativePixmap->pvAddress = surface->nativeHandle; //;CMEM_getPhys((void*)pNativePixmap->lAddress);
        LOG_DEBUG("BeagleEglImage", "Physical address = " << pNativePixmap->pvAddress);
        LOG_DEBUG("BeagleEglImage", "Logical address = " << pNativePixmap->lAddress);

        nativeSurface->eglImage = m_pfEglCreateImageKHR(m_eglDisplay,
                EGL_NO_CONTEXT, //eglContext,
                EGL_NATIVE_PIXMAP_KHR, //EGL_GL_TEXTURE_2D_KHR,
                pNativePixmap, //(void*)textureId0,//
                NULL //miplevel 0 is fine, thank you
                );

        if (nativeSurface->eglImage == 0)
        {
            LOG_DEBUG("BeagleEglImage", "could not allocate EGL Image for window");
        }
        else
        {
            glGenTextures(1, &nativeSurface->texture);
        }
    }

    void destroyClientBuffer(Surface* surface)
    {
        LOG_DEBUG("BeagleEglImage", "destroyClientBuffer");
        BeaglePlatformSurface* nativeSurface = (BeaglePlatformSurface*) surface->platform;
        if (NULL != nativeSurface && 0 != nativeSurface->eglImage)
        {
            EGLBoolean status = m_pfEglDestroyImageKHR(m_eglDisplay, nativeSurface->eglImage);
            if (!status)
            {
                LOG_ERROR("BeagleEglImage", "could not delete EGLImage");
            }

            nativeSurface->eglImage = 0;
        }
    }

private:
    PFNEGLCREATEIMAGEKHRPROC m_pfEglCreateImageKHR;
    PFNEGLDESTROYIMAGEKHRPROC m_pfEglDestroyImageKHR;
    PFNGLEGLIMAGETARGETTEXTURE2DOESPROC m_pfGLEglImageTargetTexture2DOES;
    EGLDisplay m_eglDisplay;
};

#endif /* _BEAGLEEGLIMAGE_H_ */
