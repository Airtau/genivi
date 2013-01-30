/***************************************************************************
*
* Copyright 2012 Valeo
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
#include "SurfaceGetKeyboardFocusCommand.h"
#include "ICommandExecutor.h"
#include "Scene.h"
#include "Log.h"

ExecutionResult SurfaceGetKeyboardFocusCommand::execute(ICommandExecutor* executor)
{
    RendererList* rList;
    ExecutionResult result;
    RendererListConstIterator it;
    unsigned int surfId = GraphicalObject::INVALID_ID;
    unsigned int curSurfId = GraphicalObject::INVALID_ID;

    result = ExecutionSuccess;
    rList = executor->getRendererList();
    if (rList->size())
    {
        surfId = rList->front()->getInputManager()->getKeyboardFocusSurfaceId();
        for (it = rList->begin(); (it != rList->end()) && (result == ExecutionSuccess); it++)
        {
            curSurfId = (*it)->getInputManager()->getKeyboardFocusSurfaceId();
            if (curSurfId != surfId)
            {
                result = ExecutionFailed;
            }
        }
    }

    *m_pSurfId = (result == ExecutionSuccess) ? surfId : GraphicalObject::INVALID_ID;
    return result;
}

const std::string SurfaceGetKeyboardFocusCommand::getString()
{
    std::stringstream description;
    description << "SurfaceGetKeyboardFocusCommand("
                << "pSurfId=" << m_pSurfId << "=" << *m_pSurfId
                << ")";
    return description.str();
}

