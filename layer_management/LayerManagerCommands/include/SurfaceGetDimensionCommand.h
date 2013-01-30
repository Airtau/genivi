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

#ifndef _SURFACEGETDIMENSIONCOMMAND_H_
#define _SURFACEGETDIMENSIONCOMMAND_H_

#include "ICommand.h"

class SurfaceGetDimensionCommand: public ICommand
{
public:
    /*!
     * \action    This command returns the dimension of a surface within the GENIVI LayerManagement
     * \frequency Called for rearranging graphical contents.
     * \param[in] sender process id of application that sent this command
     * \param[in] id id of surface
     * \param[in] widthRet pointer to store surface width on execution
     * \param[in] heightRet pointer to store surface height on execution
     * \ingroup Commands
     */
    SurfaceGetDimensionCommand(pid_t sender, int id, unsigned int* widthRet, unsigned int* heightRet)
    : ICommand(ExecuteSynchronous, sender)
    , m_id(id)
    , m_pWidth(widthRet)
    , m_pHeight(heightRet)
    {}

    /**
     * \brief default destructor
     */
    virtual ~SurfaceGetDimensionCommand() {}

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
    unsigned int* m_pWidth;
    unsigned int* m_pHeight;

    // for unit testing
    template <typename id_type> friend class SurfaceGetDimensionCommandEqMatcherP;
};

#endif /* _SURFACEGETDIMENSIONCOMMAND_H_ */
