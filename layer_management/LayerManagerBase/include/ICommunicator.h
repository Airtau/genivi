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

#ifndef _ICOMMUNICATOR_H_
#define _ICOMMUNICATOR_H_

class ICommandExecutor;

/**
 * Abstract Base of all Communicator plugins.
 * \defgroup CommunicatorAPI Layer Management Communicator API
 */
class ICommunicator
{
public:
    /**
     * \brief     constructor: any communicator need a executor for commands
     * \ingroup   CommunicatorAPI
     * \param[in] executor Pointer to an object to send commands to
     */
    ICommunicator(ICommandExecutor* executor);

    /**
     * \brief     default destructor
     */
    virtual ~ICommunicator() {}

    /**
     * \brief     Start communication process, i.e. start specific listening process of communication method
     * \ingroup   CommunicatorAPI
     */
    virtual bool start() = 0;

    /**
     * \brief     Stop communication. Stop sending command objects.
     * \ingroup   CommunicatorAPI
     */
    virtual void stop() = 0;

    /**
     * \brief     Process communication.
     * \ingroup   CommunicatorAPI
     * \param[in] timeout_ms timeout value in milliseconds
     */
    virtual void process(int timeout_ms) = 0;

    /**
     * \brief     Switch debug mode of this component on or off
     * \ingroup   CommunicatorAPI
     * \param[in] onoff TRUE: Turn on debug mode, FALSE: Turn off debug mode
     */
    virtual void setdebug(bool onoff) = 0;

protected:
    ICommandExecutor* m_executor;
};

inline ICommunicator::ICommunicator(ICommandExecutor* executor)
: m_executor(executor)
{
}


#endif // _ICOMMUNICATOR_H_
