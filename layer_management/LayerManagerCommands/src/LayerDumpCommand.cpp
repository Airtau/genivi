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
#include "LayerDumpCommand.h"
#include "ICommandExecutor.h"
#include "Scene.h"
#include "Log.h"

ExecutionResult LayerDumpCommand::execute(ICommandExecutor* executor)
{
	ExecutionResult result = ExecutionFailed;
    Scene& scene = *(executor->getScene());
    RendererList& m_rendererList = *(executor->getRendererList());

    bool status = false;

    LOG_INFO("LayerDumpCommand","making screenshot, output file: " << m_filename);

	if (scene.getLayer(m_id))
	{
		status = scene.isLayerInCurrentRenderOrder(m_id);
		if (!status)
		{
			LOG_WARNING("LayerDumpCommand","Requested layer: " << m_id << " does not belong to the current render order");
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
                renderer->doScreenShotOfLayer(m_filename, m_id);
            }
        }
        result = ExecutionSuccessRedraw;
    }

    return result;
}

const std::string LayerDumpCommand::getString()
{
    std::stringstream description;
    description << "LayerDumpCommand("
                << "filename=" << m_filename
                << ", id=" << m_id
                << ")";
    return description.str();
}
