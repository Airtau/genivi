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

#ifndef _X11TEXTUREFROMPIXMAP_H_
#define _X11TEXTUREFROMPIXMAP_H_

#define GL_GLEXT_PROTOTYPES
#define GLX_GLXEXT_PROTOTYPES

#include "GL/glx.h"
#include "TextureBinders/ITextureBinder.h"
#include "PlatformSurfaces/GLXPlatformSurface.h"

class X11TextureFromPixmap: public ITextureBinder
{
public:
    X11TextureFromPixmap(Display* display, GLXFBConfig pixmapConfig);

    bool bindSurfaceTexture(Surface* surface);
    bool unbindSurfaceTexture(Surface* surface);
    void createClientBuffer(Surface* surface);
    void destroyClientBuffer(Surface* surface);
    virtual PlatformSurface* createPlatformSurface(Surface* s);

private:
    Display* dpy;
    PFNGLXBINDTEXIMAGEEXTPROC glXBindTexImageEXT_func;
    PFNGLXRELEASETEXIMAGEEXTPROC glXReleaseTexImageEXT_func;
    GLXFBConfig pixmapConfig;

    static const int pixmapAttribs[];
};

#endif /* _X11TEXTUREFROMPIXMAP_H_ */
