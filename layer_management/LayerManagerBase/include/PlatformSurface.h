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

#ifndef _PLATFORMSURFACE_H_
#define _PLATFORMSURFACE_H_

class Surface;

/*
 * Contains platform specific information ie specifics for the rendering used, for instance platform specific handles
 */
class PlatformSurface
{
public:
    /**
     * Platform specific Surface information is contained in subclasses of PlatformSurface.
     * The Platformspecific implementation can gain access to platform independant attributes (position, alpha) of the corresponding surface
     * through the given reference
     */
    PlatformSurface(Surface* surface)
    : s(surface)
    {
    }

    virtual ~PlatformSurface()
    {
    }

    // Reference to platform independant surface type for access to size etc
    Surface* s;

};

#endif /* _PLATFORMSURFACE_H_ */
