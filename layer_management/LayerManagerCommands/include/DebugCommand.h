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

#ifndef _DEBUGCOMMAND_H_
#define _DEBUGCOMMAND_H_

#include "ICommand.h"

class DebugCommand : public ICommand
{
public:
    /*!
     * \action    This command sets the debug mode within the GENIVI LayerManagement
     * \frequency Used only for development and debugging.
     * \param[in] sender process id of application that sent this command
     * \param[in] onoff TRUE: enable debug mode, FALSE: disable debug mode
     * \ingroup Commands
     */
    DebugCommand(pid_t sender, bool onoff)
    : ICommand(ExecuteSynchronous, sender)
    , m_onoff(onoff)
    {}

    /**
     * \brief default destructor
     */
    virtual ~DebugCommand() {}

    /**
     * \brief Execute this command.
     * \param[in] executor Pointer to instance executing the LayerManagement COmmands
     * \return ExecutionSuccess: execution successful
     * \return ExecutionSuccessRedraw: execution successful and screen needs to be redrawn
     * \return ExecutionFailed: execution failed
     * \return ExecutionFailedRedraw: execution unsuccessful and screen needs to be redrawn
     */
    virtual ExecutionResult execute(ICommandExecutor* executor);

    /**
     * \brief Get description string for this command.
     * \return String object with description of this command object
     */
    virtual const std::string getString();

private:
    const bool m_onoff;

    // for unit testing
    template <typename onoff_type> friend class DebugCommandEqMatcherP;
};

#endif /* _DEBUGCOMMAND_H_ */
