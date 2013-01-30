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

#include "TextureBinders/X11CopyGLX.h"
#include "Surface.h"
#include "X11/Xlib.h"
#include "Log.h"

#include "GL/glx.h"

bool X11CopyGLX::bindSurfaceTexture(Surface* surface)
{
    XPlatformSurface* nativeSurface = NULL;
    Pixmap pixmap = 0;
    GLenum targetType = GL_BGRA;
    GLenum sourceType = GL_RGBA;
    if (surface != NULL )
    {
        nativeSurface = (XPlatformSurface*)surface->platform;
    }
    if( nativeSurface != NULL && surface->getNativeContent() != -1 && nativeSurface->isReadyForRendering())
    {
        pixmap = XCompositeNameWindowPixmap (dpy, surface->getNativeContent());
        if (!pixmap)
        {
            LOG_ERROR("X11CopyGLX", "didnt create pixmap!");
            return false;
        }

        nativeSurface->pixmap = pixmap;
        XImage * xim = XGetImage(dpy, nativeSurface->pixmap, 0, 0, surface->OriginalSourceWidth, surface->OriginalSourceHeight, AllPlanes, ZPixmap);
        if (xim)
        {
            glBindTexture(GL_TEXTURE_2D, nativeSurface->texture);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            if ( surface->getPixelFormat() == PIXELFORMAT_RGB888 )
            {
                targetType = GL_BGR;
                sourceType = GL_RGB;
            } 
            else if ( surface->getPixelFormat() == PIXELFORMAT_RGBA8888 )
            {
                targetType = GL_BGRA;
                sourceType = GL_RGBA;
                if (xim->depth == 24)
                {
                    /* Set alpha value */
                    int count = surface->OriginalSourceWidth*surface->OriginalSourceHeight;
                    for (int j=0;j<count; j++)
                    {
                        xim->data[j*4+3]=255;
                    }
                }
            } else {
                LOG_ERROR("X11CopyGLX","Pixelformat currently not supported : " << surface->getPixelFormat());
                XDestroyImage(xim);
                return false;
            }
            glTexImage2D(GL_TEXTURE_2D, 0, sourceType, surface->OriginalSourceWidth, surface->OriginalSourceHeight, 0, targetType, GL_UNSIGNED_BYTE, xim->data);
            XDestroyImage(xim);
            return true;
        }
    }
    return false;
}

void X11CopyGLX::createClientBuffer(Surface* surface)
{
    XPlatformSurface* nativeSurface = (XPlatformSurface*)surface->platform;
    glGenTextures(1,&nativeSurface->texture);
}

void X11CopyGLX::destroyClientBuffer(Surface* surface)
{
    XPlatformSurface* nativeSurface = (XPlatformSurface*) surface->platform;
    if (nativeSurface && nativeSurface->pixmap)
    {
          glDeleteTextures(1,&nativeSurface->texture);
          XFreePixmap(dpy, nativeSurface->pixmap);
    }
}
