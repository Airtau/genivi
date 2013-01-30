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
#include "LayerSetTypeCommand.h"
#include "ICommandExecutor.h"
#include "Scene.h"
#include "Log.h"

ExecutionResult LayerSetTypeCommand::execute(ICommandExecutor* executor)
{
    Scene& scene = *(executor->getScene());

    ExecutionResult result = ExecutionFailed;

    Layer* layer = NULL;

    layer = scene.getLayer(m_idtoSet);

    if (layer)
    {
        LOG_DEBUG("LayerSetTypeCommand", "setting type: " << m_layerType << " of layer id " << m_idtoSet);
        if (layer->setLayerType(m_layerType))
        {
            result = ExecutionSuccessRedraw;
        }
        else
        {
            result = ExecutionSuccess;
        }
    }

    return result;
}

const std::string LayerSetTypeCommand::getString()
{
    std::stringstream description;
    description << "LayerSetTypeCommand("
                << "idtoSet=" << m_idtoSet
                << ", layertype=" << m_layerType
                << ")";
    return description.str();
}
