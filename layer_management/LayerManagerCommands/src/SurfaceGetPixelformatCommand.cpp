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
#include "SurfaceGetPixelformatCommand.h"
#include "ICommandExecutor.h"
#include "Scene.h"
#include "Log.h"

ExecutionResult SurfaceGetPixelformatCommand::execute(ICommandExecutor* executor)
{
    Scene& scene = *(executor->getScene());

    ExecutionResult result = ExecutionFailed;

	Surface* surface = scene.getSurface(m_id);
	if (surface)
	{
		*m_pFormatreturn = surface->getPixelFormat();
		result = ExecutionSuccess;
	}

    return result;
}

const std::string SurfaceGetPixelformatCommand::getString()
{
    std::stringstream description;
    description << "SurfaceGetPixelformatCommand("
                << "id=" << m_id
                << ", pFormatreturn=" << m_pFormatreturn << "=" << *m_pFormatreturn
                << ")";
    return description.str();
}
