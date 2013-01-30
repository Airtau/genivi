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


#ifndef _SURFACESETKEYBOARDFOCUSCOMMAND_H_
#define _SURFACESETKEYBOARDFOCUSCOMMAND_H_


#include "ICommand.h"

class SurfaceSetKeyboardFocusCommand : public ICommand
{
    public:
        /*!
         * \action    This command sets the keyboard focus on a particular surface
         * \frequency Called whenever a surface needs to receive keyboard events
         * \param[in] sender process id of application that sent this command
         * \param[in] surfId id of surface
         * \ingroup Commands
         */
        SurfaceSetKeyboardFocusCommand(pid_t sender, unsigned int surfId)
        : ICommand(ExecuteSynchronous, sender)
        , m_surfId(surfId)
        {}

        /**
         * \brief default destructor
         */
        virtual ~SurfaceSetKeyboardFocusCommand() {}

        /**
         * \brief Execute this command.
         * \param[in] executor Pointer to instance executing the LayerManagement Commands
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
        const unsigned int m_surfId;


    // for unit testing
    template <typename surfaceid_type> friend class SetKeyboardFocusOnCommandEqMatcherP;
};

#endif  /* ! _SURFACESETKEYBOARDFOCUSCOMMAND_H_ */

