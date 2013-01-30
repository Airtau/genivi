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

#include "TextureBinders/X11Copy.h"
#include "Surface.h"
#include "X11/Xlib.h"
#include "Log.h"

X11Copy::X11Copy(Display* display)
: dpy(display)
{
}

PlatformSurface* X11Copy::createPlatformSurface(Surface* surface)
{
    LOG_DEBUG("X11Copy", "give back new platformsurface");
    return new XPlatformSurface(surface);
}

bool X11Copy::unbindSurfaceTexture(Surface* surface)
{
    (void)surface; // TODO: remove, only prevents warning
    return true;
    // TODO
}



