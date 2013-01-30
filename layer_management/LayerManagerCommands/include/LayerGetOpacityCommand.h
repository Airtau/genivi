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

#ifndef _LAYERGETOPACITYCOMMAND_H_
#define _LAYERGETOPACITYCOMMAND_H_

#include "ICommand.h"

class LayerGetOpacityCommand : public ICommand
{
public:
    /*!
     * \action    This command returns the opacity of a layer within the GENIVI LayerManagement
     * \frequency Can occur very frequently for animations.
     * \param[in] sender process id of application that sent this command
     * \param[in] id id of layer
     * \param[in] returnOpacity location to store opacity of layer on execution
     * \ingroup Commands
     */
    LayerGetOpacityCommand(pid_t sender, int id, double* returnOpacity)
    : ICommand(ExecuteSynchronous, sender)
    , m_id(id)
    , m_pReturnOpacity(returnOpacity)
    {}

    /**
     * \brief default destructor
     */
    virtual ~LayerGetOpacityCommand() {}

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
    const unsigned m_id;
    double* m_pReturnOpacity;

    // for unit testing
    template <typename id_type> friend class LayerGetOpacityCommandEqMatcherP;
};


#endif /* _LAYERGETOPACITYCOMMAND_H_ */
