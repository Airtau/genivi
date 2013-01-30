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


#ifndef _SURFACEUPDATEINPUTEVENTACCEPTANCE_H_
#define _SURFACEUPDATEINPUTEVENTACCEPTANCE_H_

#include "ilm_types.h"
#include "InputManager.h"
#include "ICommand.h"


class SurfaceUpdateInputEventAcceptance : public ICommand
{
    public:
        /*!
         * \action    This command update the list of input devices the surface
         * can accept events from. Call this method if you do not want a surface
         * to receive particular type of event (touch, keyboard, ...)
         *
         * \frequency Preferably at init. This could lead to weird results
         * if you update the acceptance at runtime, while the surface has already
         * a focus (touch, keyboard, ...)
         *
         * \param[in] sender process id of application that sent this command
         * \param[in] surfId id of surface
         * \param[in] devices Bitmask of ilmInputDevice. To set the acceptance status of one or more input device.
         *            Note that this method will only the acceptance status for the specified InputDeviced in the
         *            "devices" parameter. Not specified input device status will remain unchanged.
         * \param[in] accept if TRUE, input events from all specified devices will be accepted
         *
         * \ingroup Commands
         */
        SurfaceUpdateInputEventAcceptance(pid_t sender, unsigned int surfId, InputDevice devices, bool accept)
        : ICommand(ExecuteSynchronous, sender)
        , m_surfId(surfId)
        , m_devices(devices)
        , m_accept(accept)
        {}

        /**
         * \brief default destructor
         */
        virtual ~SurfaceUpdateInputEventAcceptance() {}

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
        const InputDevice m_devices;
        const bool m_accept;


    // for unit testing
    template <typename surfaceid_type, typename devices_type, typename acceptance_type> friend class UpdateInputEventAcceptanceOnCommandEqMatcherP3;
};

#endif  /* ! _SURFACEUPDATEINPUTEVENTACCEPTANCE_H_ */

