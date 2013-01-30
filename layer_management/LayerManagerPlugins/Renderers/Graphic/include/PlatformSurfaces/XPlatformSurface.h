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

#ifndef _XPLATFORMSURFACE_H_
#define _XPLATFORMSURFACE_H_

#include "PlatformSurface.h"
#include "Surface.h"
#include "X11/X.h"

class XPlatformSurface: public PlatformSurface
{
public:
    XPlatformSurface(Surface* surface)
    : PlatformSurface(surface)
    , isMapped(false)
    , pixmap(0)
    , texture(0)
    , m_isReadyForRendering(false)
    {
    }

    ~XPlatformSurface()
    {
    }
    
    bool enableRendering()
    {
        m_isReadyForRendering = true;
        return m_isReadyForRendering;
    }

    bool disableRendering()
    {
        m_isReadyForRendering = false;
        return m_isReadyForRendering;
    }

    bool isReadyForRendering()
    {
        return m_isReadyForRendering;
    }
    // TODO: private/protected
    bool isMapped;
    Pixmap pixmap;
    uint texture;
    bool m_isReadyForRendering;    
};

#endif /* _XPLATFORMSURFACE_H_ */
