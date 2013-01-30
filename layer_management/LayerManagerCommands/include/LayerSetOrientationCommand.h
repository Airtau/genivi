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

#ifndef _LAYERSETORIENTATIONCOMMAND_H_
#define _LAYERSETORIENTATIONCOMMAND_H_

#include "ICommand.h"
#include "OrientationType.h"

class LayerSetOrientationCommand : public ICommand
{
public:
    /*!
     * \action    This command sets the orientation of a layer within the GENIVI LayerManagement
     * \frequency Called for rearranging graphical contents.
     * \param[in] sender process id of application that sent this command
     * \param[in] id id of layer
     * \param[in] Orientation orientation of layer
     * \ingroup Commands
     */
    LayerSetOrientationCommand(pid_t sender, unsigned int id, OrientationType Orientation)
    : ICommand(ExecuteAsynchronous, sender)
    , m_id(id)
    , m_orientation(Orientation)
    {}

    /**
     * \brief default destructor
     */
    virtual ~LayerSetOrientationCommand() {}

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
    const unsigned int m_id;
    const OrientationType m_orientation;

    // for unit testing
    template <typename id_type, typename Orientation_type> friend class LayerSetOrientationCommandEqMatcherP2;
};


#endif /* _LAYERSETORIENTATIONCOMMAND_H_ */
