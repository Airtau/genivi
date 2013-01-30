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
#include "LayerAddSurfaceCommand.h"
#include "ICommandExecutor.h"
#include "Scene.h"
#include "Log.h"

ExecutionResult LayerAddSurfaceCommand::execute(ICommandExecutor* executor)
{
    Scene& scene = *(executor->getScene());

    ExecutionResult result = ExecutionFailed;

    Layer* layer = scene.getLayer(m_layerid);
    Surface* surface = scene.getSurface(m_surfaceid);

    if (layer != NULL && surface != NULL)
    {
        if (layer->addSurface(surface))
        {
            LOG_DEBUG("LayerAddSurfaceCommand","Adding surface(" << m_surfaceid << ")" << surface->getID() << " to layer(" << m_layerid << ") " << layer->getID());
            LOG_DEBUG("LayerAddSurfaceCommand", "Layer now has #surfaces:" << layer->getAllSurfaces().size());
            result = surface->hasNativeContent() ? ExecutionSuccessRedraw : ExecutionSuccess;
        }
        else
        {
            unsigned int layer_id = surface->getContainingLayerId();
            LOG_WARNING("LayerAddSurfaceCommand","surface : id [ " << m_surfaceid << " ] already belongs to layer : id [ " << layer_id << " ]");
            result = (m_layerid == layer_id) ? ExecutionSuccess : ExecutionFailed;
        }
		surface->calculateTargetDestination(layer->getSourceRegion(),layer->getDestinationRegion());
    }

    return result;
}

const std::string LayerAddSurfaceCommand::getString()
{
    std::stringstream description;
    description << "LayerAddSurfaceCommand("
                << "layerid=" << m_layerid
                << ", surfaceid=" << m_surfaceid
                << ")";
    return description.str();
}
