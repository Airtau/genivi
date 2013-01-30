/***************************************************************************
 *
 * Copyright 2010,2011 BMW Car IT GmbH
 * Copyright (c) 2012, NVIDIA CORPORATION.  All rights reserved.
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

#ifndef _BASEWINDOWSYSTEM_H_
#define _BASEWINDOWSYSTEM_H_

#include "Scene.h"

class BaseWindowSystem
{
public:
    BaseWindowSystem(Scene* pScene, InputManager* pInputManager )

    : m_pInputManager(pInputManager)
    , m_pScene(pScene)
    , m_damaged(false)
    , m_forceComposition(false)
    {
    }

    virtual ~BaseWindowSystem()
    {
    }

    virtual bool start() = 0;
    virtual void stop() = 0;
    virtual void allocatePlatformSurface(Surface *surface) = 0;
    virtual void doScreenShot(std::string fileName) = 0;
    virtual void doScreenShotOfLayer(std::string fileName, const uint id) = 0;
    virtual void doScreenShotOfSurface(std::string fileName, const uint id, const uint layer_id) = 0;

    unsigned long int mThreadId; // TODO: remove
protected:
    virtual void ClearDamage();
    InputManager* m_pInputManager;

public:
    Scene* m_pScene;
    bool m_damaged;
    bool m_forceComposition;
};

#endif /* _BASEWINDOWSYSTEM_H_ */
