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
#include "SurfaceSetChromaKeyCommand.h"
#include "ICommandExecutor.h"
#include "Scene.h"
#include "Log.h"

ExecutionResult SurfaceSetChromaKeyCommand::execute(ICommandExecutor* executor)
{
    Scene& scene = *(executor->getScene());

    ExecutionResult result = ExecutionFailed;

    Surface* surface = scene.getSurface(m_surfaceid);

    if (NULL == m_array)
    {
        if (surface)
        {
            LOG_DEBUG("SurfaceSetChromaKeyCommand","new chromakey disable for id: " << m_surfaceid);
            result = surface->setChromaKeyEnabled(false) ? ExecutionSuccessRedraw : ExecutionSuccess;
        }
    }
    else
    {
        if (surface)
        {
            LOG_DEBUG("SurfaceSetChromaKeyCommand","new chromakey (" << m_array[0] << ", " << m_array[1] << ", " << m_array[2] << ") for id: " << m_surfaceid);
            bool ansSetEnabled = surface->setChromaKeyEnabled(true);
            bool ansSetValues = surface->setChromaKey((unsigned char)m_array[0], (unsigned char)m_array[1], (unsigned char)m_array[2]);
            if ((true == ansSetEnabled) | (true == ansSetValues))
            {
                result = ExecutionSuccessRedraw;
            }
            else
            {
                result = ExecutionSuccess;
            }
        }
        delete[] m_array;
    }

    return result;
}

const std::string SurfaceSetChromaKeyCommand::getString()
{
    std::stringstream description;
    description << "SurfaceSetChromaKeyCommand("
                << "surfaceid=" << m_surfaceid
                << ", m_array=[";

    for (unsigned int i = 0; i < m_length; ++i)
    {
        description << m_array[i] << ",";
    }
    description << "], m_length=" << m_length
                << ")";
    return description.str();
}
