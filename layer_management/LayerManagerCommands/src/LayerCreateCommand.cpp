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
#include "LayerCreateCommand.h"
#include "ICommandExecutor.h"
#include "Scene.h"
#include "Log.h"

ExecutionResult LayerCreateCommand::execute(ICommandExecutor* executor)
{
    Scene& scene = *(executor->getScene());
    ExecutionResult result = ExecutionFailed;

	Layer *layer = scene.createLayer(*m_idReturn, getSenderPid());
	if (layer)
	{
		*m_idReturn = layer->getID();

		layer->setDestinationRegion(Rectangle(0, 0, m_originalWidth, m_originalHeight));
		layer->setSourceRegion(Rectangle(0, 0, m_originalWidth, m_originalHeight));
		layer->OriginalSourceWidth = m_originalWidth;
		layer->OriginalSourceHeight = m_originalHeight;

		LOG_DEBUG("LayerCreateCommand", "created layer with id: " << layer->getID() << "width: " << m_originalWidth << " height: " << m_originalHeight);
		result = ExecutionSuccess;
    }
    return result;
}

const std::string LayerCreateCommand::getString()
{
    std::stringstream description;
    description << "LayerCreateCommand("
                << "OriginalWidth=" << m_originalWidth
                << ", OriginalHeight=" << m_originalHeight
                << ", idReturn=" << m_idReturn << "=" << *m_idReturn
                << ")";
    return description.str();
}
