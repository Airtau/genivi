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
#include "SurfaceDumpCommand.h"
#include "ICommandExecutor.h"
#include "Scene.h"
#include "Log.h"

ExecutionResult SurfaceDumpCommand::execute(ICommandExecutor* executor)
{
	ExecutionResult result = ExecutionFailed;
    Scene& scene = *(executor->getScene());
    RendererList& m_rendererList = *(executor->getRendererList());

    bool status = false;
    unsigned int layer_id = 0;

    LOG_INFO("SurfaceDumpCommand","making screenshot, output file: " << m_filename);

	if (scene.getSurface(m_id))
	{
		layer_id = scene.getSurface(m_id)->getContainingLayerId();
		status = scene.isLayerInCurrentRenderOrder(layer_id);
		if (!status)
		{
			LOG_WARNING("SurfaceDumpCommand","Requested surface: " << m_id << " does not belong to a layer which is part of the current render order");
		}
	}

    if (status)
    {
        // call screen shot on all renderers for now TODO
        RendererListConstIterator iter = m_rendererList.begin();
        RendererListConstIterator iterEnd = m_rendererList.end();

        for (; iter != iterEnd; ++iter)
        {
            IRenderer* renderer = *iter;

            if (renderer)
            {
                renderer->doScreenShotOfSurface(m_filename, m_id, layer_id);
            }
        }
        result = ExecutionSuccessRedraw;
    }

    return result;
}

const std::string SurfaceDumpCommand::getString()
{
    std::stringstream description;
    description << "SurfaceDumpCommand("
                << "filename=" << m_filename
                << ", id=" << m_id
                << ")";
    return description.str();
}
