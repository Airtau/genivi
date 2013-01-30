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
#include "SurfaceSetSourceRectangleCommand.h"
#include "ICommandExecutor.h"
#include "Scene.h"
#include "Log.h"

ExecutionResult SurfaceSetSourceRectangleCommand::execute(ICommandExecutor* executor)
{
    Scene& scene = *(executor->getScene());

    ExecutionResult result = ExecutionFailed;

    Surface* surface = scene.getSurface(m_id);
    if (surface)
    {
        unsigned int layerid = surface->getContainingLayerId();
        const Rectangle rectangle(m_x, m_y, m_width, m_height);
        if (surface->setSourceRegion(rectangle))
        {
            executor->addClientNotification(surface, ILM_NOTIFICATION_SOURCE_RECT);
            result = ExecutionSuccessRedraw;
        }
        else
        {
            result = ExecutionSuccess;
        }
        if ( layerid != Surface::INVALID_ID ) 
        {
            surface->calculateTargetDestination(scene.getLayer(layerid)->getSourceRegion(),scene.getLayer(layerid)->getDestinationRegion());
        }
    }

    return result;
}

const std::string SurfaceSetSourceRectangleCommand::getString()
{
    std::stringstream description;
    description << "SurfaceSetSourceRectangleCommand("
                << "id=" << m_id
                << ", x=" << m_x
                << ", y=" << m_y
                << ", width=" << m_width
                << ", height=" << m_height
                << ")";
    return description.str();
}
