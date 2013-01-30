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

#define GL_GLEXT_PROTOTYPES
#define GLX_GLXEXT_PROTOTYPES

#include "TextureBinders/X11TextureFromPixmap.h"
#include "X11/extensions/Xcomposite.h"
#include "Log.h"

const int X11TextureFromPixmap::pixmapAttribs[] = {
        GLX_TEXTURE_TARGET_EXT, GLX_TEXTURE_2D_EXT,
        GLX_TEXTURE_FORMAT_EXT, GLX_TEXTURE_FORMAT_RGBA_EXT, None
};

X11TextureFromPixmap::X11TextureFromPixmap(Display* display, GLXFBConfig pixmapConfig)
: dpy(display)
, pixmapConfig(pixmapConfig)
{
    glXBindTexImageEXT_func = (PFNGLXBINDTEXIMAGEEXTPROC) glXGetProcAddress((GLubyte *) "glXBindTexImageEXT");
    glXReleaseTexImageEXT_func = (PFNGLXRELEASETEXIMAGEEXTPROC)    glXGetProcAddress((GLubyte*) "glXReleaseTexImageEXT");

    if (!glXBindTexImageEXT_func || !glXReleaseTexImageEXT_func)
    {
        LOG_ERROR("X11TextureFromPixmap", "glXGetProcAddress failed");
    }
}

bool X11TextureFromPixmap::bindSurfaceTexture(Surface* surface)
{
    GLXPlatformSurface* nativeSurface = (GLXPlatformSurface*)surface->platform;
    if (nativeSurface)
    {
        glXBindTexImageEXT_func(dpy, nativeSurface->glxPixmap, GLX_FRONT_LEFT_EXT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        return true;
    }
   return false;
}

bool X11TextureFromPixmap::unbindSurfaceTexture(Surface* surface)
{
    GLXPlatformSurface* nativeSurface = (GLXPlatformSurface*)surface->platform;
    if (nativeSurface && nativeSurface->isReadyForRendering())
    {
        glXReleaseTexImageEXT_func(dpy, nativeSurface->glxPixmap, GLX_FRONT_LEFT_EXT);
        return true;
    }
    return false;
}

void X11TextureFromPixmap::createClientBuffer(Surface* surface){
    LOG_DEBUG("X11TextureFromPixmap", "creating clientBuffer for window: " << surface->getNativeContent());
    GLXPlatformSurface* platformSurface = (GLXPlatformSurface*)surface->platform;
    if (NULL!=platformSurface)
    {
        Pixmap pixmap = 0;
        LOG_DEBUG("X11TextureFromPixmap", "get X Pixmap");
        pixmap= XCompositeNameWindowPixmap (dpy, surface->getNativeContent());
        if (pixmap==0)
        {
            LOG_ERROR("X11TextureFromPixmap", "didnt create pixmap!");
        }
        XSync(dpy, 0);

        GLXPixmap glxPixmap = 0;
        LOG_DEBUG("X11TextureFromPixmap", "creating glx pixmap from xpixmap");
        glxPixmap = glXCreatePixmap(dpy, pixmapConfig, pixmap, pixmapAttribs);
        if (glxPixmap ==0)
        {
            LOG_DEBUG("X11TextureFromPixmap", "could not allocate glxpixmap for window");
        }
        platformSurface->glxPixmap = glxPixmap;
        platformSurface->pixmap = pixmap;
    }
}

PlatformSurface* X11TextureFromPixmap::createPlatformSurface(Surface* surface)
{
    return new GLXPlatformSurface(surface);
}

void X11TextureFromPixmap::destroyClientBuffer(Surface* surface)
{
    LOG_ERROR("X11TextureFromPixmap", "destroying clientBuffer");
    GLXPlatformSurface* platformSurface = (GLXPlatformSurface*)surface->platform;
    LOG_DEBUG("X11TextureFromPixmap", "removing GLX Pixmap");
    if (NULL!=platformSurface)
    {
        //glXReleaseTexImageEXT_func(dpy, x11surf->glxPixmap, GLX_FRONT_LEFT_EXT);
        glXDestroyPixmap(dpy, platformSurface->glxPixmap);
        platformSurface->glxPixmap = 0;
    }
    XSync(dpy, 0);

    LOG_DEBUG("X11TextureFromPixmap", "removing X Pixmap");
    if (NULL!=platformSurface)
    {
        XFreePixmap(dpy, platformSurface->pixmap);
        platformSurface->pixmap = 0;
    }
}
