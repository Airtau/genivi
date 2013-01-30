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
#include "ScreenSetRenderOrderCommand.h"
#include "ICommandExecutor.h"
#include "Scene.h"
#include "Log.h"

ExecutionResult ScreenSetRenderOrderCommand::execute(ICommandExecutor* executor)
{
    Scene& scene = *(executor->getScene());
    ExecutionResult result = ExecutionFailed;

    // check for doubles
    for (unsigned int i = 0; i < m_length; i++)
    {
        for (unsigned int c = i + 1; c < m_length; c++)
        {
            if (m_array[i] == m_array[c])
            {
                // doubles not allowed here
                return ExecutionFailed;
            }
        }
    }

    // taken out because other software currently expects partial execution of command,
    // i.e. the layers that exist are added to the render order

    //        // check that all layers to be added exist
    //        for (unsigned int i=0;i<length;i++){
    //            Layer* l = layerlist.getLayer(array[i]);
    //            if (NULL==l)
    //                return false;
    //        } // TODO insert again later

    if (scene.getCurrentRenderOrder(m_screenID).size() != 0)
    {
        IRenderer* renderer = *((executor->getRendererList())->begin());
        renderer->forceCompositionWindowSystem();
        result = ExecutionSuccessRedraw;
    }

    scene.getCurrentRenderOrder(m_screenID).clear();

    LOG_DEBUG("ScreenSetRenderOrderCommand", "Length to set: " << m_length);

    if (m_length == 0)
    {
        result = ExecutionSuccessRedraw;
    }

    for (unsigned int i = 0; i < m_length; i++)
    {
        LOG_DEBUG("ScreenSetRenderOrderCommand", "Trying to add layer: " << m_array[i] << " to current render order");
        Layer* layer = scene.getLayer(m_array[i]);
        if (layer)
        {
            LOG_DEBUG("ScreenSetRenderOrderCommand", "Adding Layer: " << m_array[i] << " to current render order");
            scene.getCurrentRenderOrder(m_screenID).push_back(layer);
            result = ExecutionSuccessRedraw;
        }
    }

    return result;
}

const std::string ScreenSetRenderOrderCommand::getString()
{
    std::stringstream description;
    description << "ScreenSetRenderOrderCommand("
                << "m_screenID=" << m_screenID
                << ", m_array=[";

    for (unsigned int i = 0; i < m_length; ++i)
    {
        description << m_array[i] << ",";
    }
    description << "], m_length=" << m_length
                << ")";
    return description.str();
}
