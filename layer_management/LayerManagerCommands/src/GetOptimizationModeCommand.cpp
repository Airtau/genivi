/***************************************************************************
*
* Copyright 2010,2011 BMW Car IT GmbH
* Copyright (c) 2012, NVIDIA CORPORATION.  All rights reserved.
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
#include "GetOptimizationModeCommand.h"
#include "ICommandExecutor.h"
#include "Scene.h"
#include "Log.h"


ExecutionResult GetOptimizationModeCommand::execute(ICommandExecutor* executor)
{
    RendererList& m_rendererList = *(executor->getRendererList());
    RendererListConstIterator iter = m_rendererList.begin();
    RendererListConstIterator iterEnd = m_rendererList.end();

    for (; iter != iterEnd; ++iter)
    {
        IRenderer* renderer = *iter;

        if (renderer)
        {
            return renderer->getOptimizationMode(m_id, m_pReturnMode) ? ExecutionSuccess : ExecutionFailed;
        }
    }

    return ExecutionFailed;
}

const std::string GetOptimizationModeCommand::getString()
{
    std::stringstream description;
    description << "GetOptimizationModeCommand("
                << "id=" << (int)m_id
                << ", pReturnMode=" << (int)(*m_pReturnMode)
                << ")";
    return description.str();
}
