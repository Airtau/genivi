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
#include "LayerSetChromaKeyCommand.h"
#include "ICommandExecutor.h"
#include "Scene.h"
#include "Log.h"

ExecutionResult LayerSetChromaKeyCommand::execute(ICommandExecutor* executor)
{
    Scene& scene = *(executor->getScene());

    ExecutionResult result = ExecutionFailed;

    Layer* layer = scene.getLayer(m_layerid);

    if (NULL == m_array)
    {
        if (layer)
        {
            LOG_DEBUG("LayerSetChromaKeyCommand","new chromakey disable for id: " << m_layerid);
            result = layer->setChromaKeyEnabled(false) ? ExecutionSuccessRedraw : ExecutionSuccess;
        }
    }
    else
    {
        if (layer)
        {
            LOG_DEBUG("LayerSetChromaKeyCommand","new chromakey ("
                << m_array[0] << ", " << m_array[1] << ", " << m_array[2] << ") for id: " << m_layerid);
            bool ansSetEnabled = layer->setChromaKeyEnabled(true);
            bool ansSetValues = layer->setChromaKey((unsigned char)m_array[0], (unsigned char)m_array[1], (unsigned char)m_array[2]);
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

const std::string LayerSetChromaKeyCommand::getString()
{
    std::stringstream description;
    description << "LayerSetChromaKeyCommand("
                << "layerid=" << m_layerid
                << ", m_array=[";

    for (unsigned int i = 0; i < m_length; ++i)
    {
        description << m_array[i] << ",";
    }
    description << "], m_length=" << m_length
                << ")";
    return description.str();
}
