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
#include "CommitCommand.h"
#include "ICommandExecutor.h"
#include "Scene.h"
#include "Log.h"

ExecutionResult CommitCommand::execute(ICommandExecutor* executor)
{
    bool success = true;
    bool redraw = false;

    unsigned int commitCommandPid = getSenderPid();

    CommandList& clientCommandQueue = executor->getEnqueuedCommands(commitCommandPid);
    CommandListIterator iter = clientCommandQueue.begin();
    CommandListIterator iterEnd = clientCommandQueue.end();

    for (; iter != iterEnd; ++iter)
    {
        ICommand* command = (*iter);
        if (command)
        {
            bool commandExecuteSuccess = true;
            ExecutionResult result = command->execute(executor);

            if (result == ExecutionFailed || result == ExecutionFailedRedraw)
            {
                success = false;
                commandExecuteSuccess = false;
            }

            if (result == ExecutionFailedRedraw || result == ExecutionSuccessRedraw)
            {
                redraw = true;
            }

            unsigned int pid = command->getSenderPid();
            LOG_INFO("CommitCommand", "executed " << command->getString()
                    << " from " << executor->getSenderName(pid) << "(" << pid << ")"
                    << (commandExecuteSuccess ? "+" : "-"));
            delete command;
        }
    }
    clientCommandQueue.clear();

    ExecutionResult returnValue = ExecutionFailed;

    if (success)
    {
        if (redraw)
        {
            returnValue = ExecutionSuccessRedraw;
        }
        else
        {
            returnValue = ExecutionSuccess;
        }
    }
    else
    {
        if (redraw)
        {
            returnValue = ExecutionFailedRedraw;
        }
        else
        {
            returnValue = ExecutionFailed;
        }
    }
    return returnValue;
}

const std::string CommitCommand::getString()
{
    return "CommitCommand()";
}
