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

#ifndef _LAYERREMOVECOMMAND_H_
#define _LAYERREMOVECOMMAND_H_

#include "ICommand.h"

class LayerRemoveCommand : public ICommand
{
public:
    /*!
     * \action    This command removes a layer within the GENIVI LayerManagement
     * \frequency The output of several applications is grouped into layers so they
     *            can be adjusted together. This means there will be less layers
     *            than surfaces. A small configuration might create a layer for
     *            everything concerning OEM branding, one layer for third party
     *            applications and one layer for status applications.
     * \param[in] sender process id of application that sent this command
     * \param[in] objectID id of layer
     * \ingroup Commands
     */
    LayerRemoveCommand(pid_t sender, unsigned int objectID)
    : ICommand(ExecuteSynchronous, sender)
    , m_idToRemove(objectID)
    {}

    /**
     * \brief default destructor
     */
    virtual ~LayerRemoveCommand() {}

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
    const unsigned int m_idToRemove;

    // for unit testing
    template<typename idToRemove_type> friend class LayerRemoveCommandEqMatcherP;
};

#endif /* _LAYERREMOVECOMMAND_H_ */
