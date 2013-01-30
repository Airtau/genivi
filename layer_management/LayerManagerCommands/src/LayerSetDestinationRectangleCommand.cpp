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
#include "LayerSetDestinationRectangleCommand.h"
#include "ICommandExecutor.h"
#include "GraphicalSurface.h"
#include "Scene.h"
#include <sstream>

ExecutionResult LayerSetDestinationRectangleCommand::execute(ICommandExecutor* executor)
{
    Scene& scene = *(executor->getScene());

    ExecutionResult result = ExecutionFailed;

    Layer* layer = scene.getLayer(m_id);

    if (layer)
    {
        const Rectangle rectangle(m_x, m_y, m_width, m_height);
        if (layer->setDestinationRegion(rectangle))
        {
            result = ExecutionSuccessRedraw;
            executor->addClientNotification(layer, ILM_NOTIFICATION_DEST_RECT);
        }
        else
        {
            result = ExecutionSuccess;
        }

        layer->applySurfaceTransform();
    }

    return result;
}

const std::string LayerSetDestinationRectangleCommand::getString()
{
    std::stringstream description;
    description << "LayerSetDestinationRectangleCommand("
                << "id=" << m_id
                << ", x=" << m_x
                << ", y=" << m_y
                << ", width=" << m_width
                << ", height=" << m_height
                << ")";
    return description.str();
}
