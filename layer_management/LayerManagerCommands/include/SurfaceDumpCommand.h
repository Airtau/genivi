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

#ifndef _SURFACEDUMPCOMMAND_H_
#define _SURFACEDUMPCOMMAND_H_

#include "ICommand.h"
#include <string>

class SurfaceDumpCommand: public ICommand
{
public:
    /*!
     * \action    This command stores a bitmap file with the graphical content of a surface within the GENIVI LayerManagement
     * \frequency Used for surface management.
     * \param[in] sender process id of application that sent this command
     * \param[in] givenfilename path and filename for bitmap file
     * \param[in] id id of surface
     * \ingroup Commands
     */
    SurfaceDumpCommand(pid_t sender, char* givenfilename, unsigned int id = 0)
    : ICommand(ExecuteSynchronous, sender)
    , m_filename(givenfilename)
    , m_id(id)
    {}

    /**
     * \brief default destructor
     */
    virtual ~SurfaceDumpCommand() {}

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
    std::string m_filename;
    const unsigned int m_id;
};

#endif /* _SURFACEDUMPCOMMAND_H_ */
