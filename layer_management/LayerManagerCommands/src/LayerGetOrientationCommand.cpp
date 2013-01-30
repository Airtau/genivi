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
#include "LayerGetOrientationCommand.h"
#include "ICommandExecutor.h"
#include "Scene.h"
#include <sstream>

ExecutionResult LayerGetOrientationCommand::execute(ICommandExecutor* executor)
{
    Scene& scene = *(executor->getScene());

    ExecutionResult result = ExecutionFailed;

    Layer* layer = scene.getLayer(m_id);

    if (layer)
    {
        *m_pReturnOrientation = layer->getOrientation();
        result = ExecutionSuccess;
    }

    return result;
}

const std::string LayerGetOrientationCommand::getString()
{
    std::stringstream description;
    description << "LayerGetOrientationCommand("
                << "id=" << m_id
                << ", pReturnOrientation=" << m_pReturnOrientation << "=" << *m_pReturnOrientation
                << ")";
    return description.str();
}
