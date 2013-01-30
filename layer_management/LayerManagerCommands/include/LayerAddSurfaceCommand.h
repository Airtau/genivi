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

#ifndef _LAYERADDSURFACECOMMAND_H_
#define _LAYERADDSURFACECOMMAND_H_

#include "ICommand.h"

class LayerAddSurfaceCommand : public ICommand
{
public:
    /*!
     * \action    This command adds a surface to a layer within the GENIVI LayerManagement
     * \frequency Typically surfaces will be added to one or more layers once in their life
     *            cycle. So this will typically be called at least once for every surface
     *            created.
     * \param[in] sender process id of application that sent this command
     * \param[in] layerid id of layer
     * \param[in] surfaceid id of surface
     * \ingroup Commands
     */
    LayerAddSurfaceCommand(pid_t sender, unsigned int layerid, unsigned int surfaceid)
    : ICommand(ExecuteAsynchronous, sender)
    , m_layerid(layerid)
    , m_surfaceid(surfaceid)
    {}

    /**
     * \brief default destructor
     */
    virtual ~LayerAddSurfaceCommand() {}

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
    const unsigned int m_layerid;
    const unsigned int m_surfaceid;

    // for unit testing
    template <typename surfaceid_type, typename layerid_type> friend class LayerAddSurfaceCommandEqMatcherP2;
};


#endif /* _LAYERADDSURFACECOMMAND_H_ */
