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
#include "LayerSetOpacityCommand.h"
#include "ICommandExecutor.h"
#include "Scene.h"
#include "Log.h"

ExecutionResult LayerSetOpacityCommand::execute(ICommandExecutor* executor)
{
    Scene& scene = *(executor->getScene());

    ExecutionResult result = ExecutionFailed;

    Layer* layer = scene.getLayer(m_id);

    if (layer)
    {
        LOG_DEBUG("LayerSetOpacityCommand","new opacity " << m_opacity << " for id: " << m_id);
        if (layer->setOpacity(m_opacity))
        {
            result = ExecutionSuccessRedraw;
            executor->addClientNotification(layer, ILM_NOTIFICATION_OPACITY);
        }
        else
        {
            result = ExecutionSuccess;
        }
    }

    return result;
}

const std::string LayerSetOpacityCommand::getString()
{
    std::stringstream description;
    description << "LayerSetOpacityCommand("
                << "id=" << m_id
                << ", opacity=" << m_opacity
                << ")";
    return description.str();
}
