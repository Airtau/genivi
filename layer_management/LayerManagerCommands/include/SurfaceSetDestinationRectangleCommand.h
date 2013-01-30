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

#ifndef _SURFACESETDESTINATIONRECTANGLECOMMAND_H_
#define _SURFACESETDESTINATIONRECTANGLECOMMAND_H_

#include "ICommand.h"

class SurfaceSetDestinationRectangleCommand : public ICommand
{
public:
    /*!
     * \action    This command sets the destination region of a surface within the GENIVI LayerManagement
     * \frequency Called to rearrange applications output.
     * \param[in] sender process id of application that sent this command
     * \param[in] id id of surface
     * \param[in] x x position of surface on layer
     * \param[in] y y position of surface on layer
     * \param[in] width width of surface on layer
     * \param[in] height height of surface on layer
     * \ingroup Commands
     */
    SurfaceSetDestinationRectangleCommand(pid_t sender, int id, unsigned int x, unsigned int y, unsigned int width, unsigned int height)
    : ICommand(ExecuteAsynchronous, sender)
    , m_id(id)
    , m_x(x)
    , m_y(y)
    , m_width(width)
    , m_height(height)
    {}

    /**
     * \brief default destructor
     */
    virtual ~SurfaceSetDestinationRectangleCommand() {}

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
    const unsigned int m_x;
    const unsigned int m_y;
    const unsigned int m_width;
    const unsigned int m_height;

    // for unit testing
    template <typename id_type, typename x_type, typename y_type, typename width_type, typename height_type> friend class SurfaceSetDestinationRectangleCommandEqMatcherP5;
};


#endif /* _SURFACESETDESTINATIONRECTANGLECOMMAND_H_ */
