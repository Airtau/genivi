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

#ifndef _X11EGLIMAGE_H_
#define _X11EGLIMAGE_H_

#include "TextureBinders/ITextureBinder.h"
#include "X11/Xlib.h"
#include "X11/Xutil.h"
#include "X11/extensions/Xcomposite.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

class X11EglImage: public ITextureBinder
{
public:
    virtual PlatformSurface* createPlatformSurface(Surface* surface);

    X11EglImage(EGLDisplay eglDisplay, Display* x11display);
    bool bindSurfaceTexture(Surface* surface);
    bool unbindSurfaceTexture(Surface* surface);
    void createClientBuffer(Surface* surface);
    void destroyClientBuffer(Surface* surface);

private:
    PFNEGLCREATEIMAGEKHRPROC m_pfEglCreateImageKHR;
    PFNEGLDESTROYIMAGEKHRPROC m_pfEglDestroyImageKHR;
    PFNGLEGLIMAGETARGETTEXTURE2DOESPROC m_pfGLEglImageTargetTexture2DOES;
    EGLDisplay m_eglDisplay;
    Display* m_x11display;
};

#endif /* _X11EGLIMAGE_H_ */
