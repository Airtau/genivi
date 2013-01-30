/***************************************************************************
 *
 * Copyright 2010,2011 BMW Car IT GmbH
 * Copyright (C) 2012 DENSO CORPORATION and Robert Bosch Car Multimedia Gmbh
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

#ifndef _LMSCREEN_H_
#define _LMSCREEN_H_

#include "Layer.h"
#include "LayerList.h"
#include <pthread.h>
#include <string.h>

/*
 * A Screen for LayerManagement
 */
class LmScreen
{
    friend class Scene;

public:
    LmScreen();
    LmScreen(unsigned int id, const char* deviceName);

    virtual ~LmScreen();

    virtual unsigned int getID();
    virtual LayerList& getCurrentRenderOrder();
    char* getDeviceName();

protected:
    unsigned int m_id;

private:
    LayerList m_currentRenderOrder;
    char* m_deviceName;
};

inline LmScreen::LmScreen()
: m_id(0)
{
    m_deviceName = NULL;
}

inline LmScreen::LmScreen(unsigned int id, const char* deviceName)
: m_id(id)
{
    m_deviceName = new char[strlen(deviceName)];
    strcpy(m_deviceName, deviceName);
}

inline LmScreen::~LmScreen()
{
    if (NULL != m_deviceName)
    {
        delete m_deviceName;
        m_deviceName = NULL;
    }
}

inline LayerList& LmScreen::getCurrentRenderOrder() // TODO: const
{
    return m_currentRenderOrder;
}

inline unsigned int LmScreen::getID()
{
    return m_id;
}

inline char* LmScreen::getDeviceName()
{
    return m_deviceName;
}

#endif /* _LMSCREEN_H_ */
