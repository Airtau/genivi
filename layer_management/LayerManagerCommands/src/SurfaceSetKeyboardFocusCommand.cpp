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

#include <sstream>

#include "RendererList.h"
#include "SurfaceSetKeyboardFocusCommand.h"
#include "ICommandExecutor.h"
#include "InputManager.h"

ExecutionResult SurfaceSetKeyboardFocusCommand::execute(ICommandExecutor* executor)
{
    RendererList* rList;
    ExecutionResult result;
    RendererListConstIterator it;

    result = ExecutionSuccess;
    rList = executor->getRendererList();
    for (it = rList->begin(); (it != rList->end()) && (result == ExecutionSuccess); it++)
    {
        result = (*it)->getInputManager()->setKeyboardFocusOn(m_surfId) ? ExecutionSuccess : ExecutionFailed;
    }
    
    return result;
}

const std::string SurfaceSetKeyboardFocusCommand::getString()
{
    std::stringstream description;
    
    description << "SurfaceSetKeyboardFocusCommand("
                << "m_surfId=" << m_surfId
                << ")";
    return description.str();
}



