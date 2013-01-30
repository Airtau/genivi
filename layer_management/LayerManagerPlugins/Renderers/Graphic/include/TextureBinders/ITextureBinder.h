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

#ifndef _ITEXTUREBINDER_H_
#define _ITEXTUREBINDER_H_

#include "Surface.h"

class ITextureBinder
{
public:
    virtual ~ITextureBinder()
    {
    }

    virtual bool bindSurfaceTexture(Surface*s) = 0;
    virtual bool unbindSurfaceTexture(Surface*s) = 0;
    virtual void createClientBuffer(Surface*s) = 0;
    virtual void destroyClientBuffer(Surface*s) = 0;
    virtual PlatformSurface* createPlatformSurface(Surface* s) = 0;
};

#endif /* _ITEXTUREBINDER_H_ */
