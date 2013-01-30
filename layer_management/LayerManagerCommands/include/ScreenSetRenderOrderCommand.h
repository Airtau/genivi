/***************************************************************************
 *
 * Copyright 2010,2011 BMW Car IT GmbH
 * Copyright (C) 2012 DENSO CORPORATION and Robert Bosch Car Multimedia Gmbh
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

#ifndef _SCREENSETRENDERORDERCOMMAND_H_
#define _SCREENSETRENDERORDERCOMMAND_H_

#include "ICommand.h"

class ScreenSetRenderOrderCommand: public ICommand
{
public:
    /*!
     * \action    This command sets the render order of layers within the GENIVI LayerManagement
     * \frequency Called for rearranging graphical contents.
     * \param[in] sender process id of application that sent this command
     * \param[in] screenID ID of screen
     * \param[in] array array of layer ids
     * \param[in] length length of array provided in parameter array
     * \ingroup Commands
     */
    ScreenSetRenderOrderCommand(pid_t sender, unsigned int screenID, unsigned int* array, unsigned int length)
    : ICommand(ExecuteAsynchronous, sender)
    , m_screenID(screenID)
    , m_array(array)
    , m_length(length)
    {}

    /**
     * \brief default destructor
     */
    virtual ~ScreenSetRenderOrderCommand() {}

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
    unsigned int m_screenID;
    unsigned int* m_array;
    const unsigned int m_length;
};

#endif /* _SCREENSETRENDERORDERCOMMAND_H_ */
