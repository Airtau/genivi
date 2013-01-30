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

#ifndef _WAYLANDPLATFORMSURFACE_H_
#define _WAYLANDPLATFORMSURFACE_H_

#include "PlatformSurface.h"
#include "Surface.h"

class WaylandPlatformSurface: public PlatformSurface
{
public:
    unsigned short connectionId;
    unsigned short surfaceId;

    WaylandPlatformSurface(Surface* surface)
    : PlatformSurface(surface)
    , connectionId(0)
    , surfaceId(0)
    , texture(0)
    , m_isReadyForRendering(false)
    {
    }

    ~WaylandPlatformSurface()
    {
    }
    
    bool enableRendering()
    {
        m_isReadyForRendering = true;
	return true;
    }

    bool disableRendering()
    {
        m_isReadyForRendering = false;
	return true;
    }

    bool isReadyForRendering()
    {
        return m_isReadyForRendering;
    }
    uint texture;
    bool m_isReadyForRendering;    
};

#endif /* _WAYLANDPLATFORMSURFACE_H_ */
