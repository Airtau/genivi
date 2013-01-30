/***************************************************************************
*
* Copyright 2012 Valeo
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

#ifndef _SURFACEGETKEYBOARDFOCUSCOMMAND_H_
#define _SURFACEGETKEYBOARDFOCUSCOMMAND_H_

#include "ICommand.h"

class SurfaceGetKeyboardFocusCommand : public ICommand
{
public:
    /*!
     * \action    This command returns the identifier of the surface which currently hold the keyboard focus
     * \frequency Whenever it is needed
     * \param[in] sender process id of application that sent this command
     * \param[out] pSurfId id of surface
     * \ingroup Commands
     */
    SurfaceGetKeyboardFocusCommand(pid_t sender, unsigned int *pSurfId)
    : ICommand(ExecuteSynchronous, sender)
    , m_pSurfId(pSurfId)
    {}

    /**
     * \brief default destructor
     */
    virtual ~SurfaceGetKeyboardFocusCommand() {}

    /**
     * \brief Execute this command.
     * \param[in] executor Pointer to instance executing the LayerManagement COmmands
     * \return ExecutionSuccess: execution successful
     * \return ExecutionFailed: execution failed
     */
    virtual ExecutionResult execute(ICommandExecutor* executor);

    /**
     * \brief Get description string for this command.
     * \return String object with description of this command object
     */
    virtual const std::string getString();

private:
    unsigned int* m_pSurfId;


    // for unit testing
    friend class GetKeyboardFocusSurfaceIdCommandEqMatcher;
};


#endif /* _SURFACEGETKEYBOARDFOCUSCOMMAND_H_ */
