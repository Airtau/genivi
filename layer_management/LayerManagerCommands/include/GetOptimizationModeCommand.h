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

#ifndef _GETOPTIMIZATIONMODECOMMAND_H_
#define _GETOPTIMIZATIONMODECOMMAND_H_

#include "ICommand.h"
#include "OptimizationType.h"

class GetOptimizationModeCommand : public ICommand
{
public:
    /*!
     * \action    This command returns the current mode for the specified
     * optimization.
     * \param[in] sender client process id that sent this command
     * \param[in] id id of optimization
     * \param[in] returnMode location to store mode of optimization on execution
     * \ingroup Commands
     */
    GetOptimizationModeCommand(pid_t sender,OptimizationType id, OptimizationModeType* returnMode)
    : ICommand(ExecuteSynchronous, sender)
    , m_id(id)
    , m_pReturnMode(returnMode)
    {}
    /**
     * \brief default destructor
     */
    virtual ~GetOptimizationModeCommand() {}

    /**
     * \brief Execute this command.
     * \param[in] executor Pointer to instance executing the LayerManagement Commands
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
    const OptimizationType m_id;
    OptimizationModeType* m_pReturnMode;
};

#endif /* _GETOPTIMIZATIONMODECOMMAND_H_ */
