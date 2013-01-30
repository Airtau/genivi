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

#ifndef _X11COPYGLES_H_
#define _X11COPYGLES_H_

#include "TextureBinders/X11Copy.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "PlatformSurfaces/XPlatformSurface.h"
#include <X11/extensions/Xcomposite.h>
#include <EGL/egl.h>

class X11CopyGLES: public X11Copy
{
public:
    X11CopyGLES(EGLDisplay eglDisplay, Display* display) : X11Copy(display)
    {

        // pseudo require EGL to have been initialised
        // we dont really need the handle
        // remove compiler Warning
        (void)eglDisplay; // TODO
    };
    void swapPixmap(unsigned char* src,unsigned char* dest,unsigned int width,unsigned int height,bool swaprgb,bool includeAlpha);
    bool bindSurfaceTexture(Surface* surface);
    void createClientBuffer(Surface* surface);
    void destroyClientBuffer(Surface* surface);

};

#endif /* _X11COPYGLES_H_ */
