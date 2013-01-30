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

#ifndef _COMMANDEXECUTOR_H_
#define _COMMANDEXECUTOR_H_

#include "RendererList.h"
#include "CommunicatorList.h"
#include "SceneProviderList.h"
#include "HealthMonitorList.h"
#include "ApplicationReferenceMap.h"
#include "LayerType.h"
#include "ObjectType.h"
#include "CommandList.h"
#include "NotificationQueue.h"
#include <pthread.h>

class ICommand;
class Scene;
class Configuration;

/**
 * \brief Interface for LayerManagement Command Executors
 *
 * \defgroup ServiceAPI Layer Management Service API
 *
 * Objects who implement this interface can be used to have command objects
 * executed. Communication classes must only know this interface of a class to
 * be able to pass along command objects.
 *
 * \note This interface is used to reduce dependency of communicators on the
 * main layermanagement component
 */
class ICommandExecutor
{
public:
    /**
     * \brief default destructor
     * \ingroup ServiceAPI
     */
    virtual ~ICommandExecutor() {}

    /**
     * \brief Have a command processed.
     * \ingroup ServiceAPI
     * \param[in] commandToBeExecuted The command to be processed
     * \return TRUE: execution of command successful
     * \return FALSE: execution of command failed
     */
    virtual bool execute(ICommand* commandToBeExecuted) = 0;

    /**
     * \brief start layer management
     * \ingroup ServiceAPI
     * \return TRUE: start management successful
     * \return FALSE: start management failed
     */
    virtual bool startManagement() = 0;

    /**
     * \brief stop layer management
     * \ingroup ServiceAPI
     * \return TRUE: stopped management successfully
     * \return FALSE: stopping management failed
     */
    virtual bool stopManagement() = 0;

    /**
     * \brief get handle to scene data structure
     * \ingroup ServiceAPI
     * \return Pointer to the internal scene data structure
     */
    virtual Scene* getScene(void) = 0;

    // renderer management

    /**
     * \brief get list of renderer plugins currently used
     * \ingroup ServiceAPI
     * \return Pointer to list of renderer plugins
     */
    virtual RendererList* getRendererList(void) = 0;

    // communicator management

    /**
     * \brief get list of communicator plugins currently used
     * \ingroup ServiceAPI
     * \return Pointer to internal list of communicators
     */
    virtual CommunicatorList* getCommunicatorList(void) = 0;

    // scene provider management

    /**
     * \brief get list of scene provider plugins currently used
     * \ingroup ServiceAPI
     * \return Pointer to internal list of communicators
     */
    virtual SceneProviderList* getSceneProviderList(void) = 0;

    // health monitor management

    /**
     * \brief get list of health monitor plugins currently used
     * \ingroup ServiceAPI
     * \return Pointer to internal list of health monitors
     */
    virtual HealthMonitorList* getHealthMonitorList(void) = 0;

    // client application management

    /**
     * \brief get map of currently registered applications
     * \ingroup ServiceAPI
     * \return map of currently registered applications
     */
    virtual ApplicationReferenceMap* getApplicationReferenceMap(void) = 0;

    /**
     * \brief add application to list of currently registered applications
     * \ingroup ServiceAPI
     * \param[in] client handle to connected client application
     * \param[in] applicationReference pointer to application object
     */
    virtual void addApplicationReference(t_ilm_client_handle client, IApplicationReference* applicationReference) = 0;

    /**
     * \brief remove application from list of currently registered applications
     * \ingroup ServiceAPI
     * \param[in] client handle to connected client application
     */
    virtual void removeApplicationReference(t_ilm_client_handle client) = 0;

    /**
     * \brief get pid of a connected application
     * \ingroup ServiceAPI
     * \param[in] client client handle to get process id for
     */
    virtual t_ilm_uint getSenderPid(t_ilm_client_handle client) = 0;

    /**
     * \brief get name of a connected application
     * \ingroup ServiceAPI
     * \param[in] client client handle to get process name for
     */
    virtual const char* getSenderName(t_ilm_client_handle client) = 0;

    /**
     * \brief get name of a connected application
     * \ingroup ServiceAPI
     * \param[in] pid process id to get process name for
     */
    virtual const char* getSenderName(unsigned int pid) = 0;

    /**
     * \brief get capabilities of layer type
     * \ingroup ServiceAPI
     * \param[in] layertype layer type
     * \return bitmask with capability flags set according to layer type capabilities
     * \todo removable, use default command processing
     */
    virtual unsigned int getLayerTypeCapabilities(const LayerType layertype) const = 0;

    /**
     * \brief get number of supported hardware layers for screen
     * \ingroup ServiceAPI
     * \param[in] screenID id of screen
     * \return number of supported hardware layers for screen
     * \todo removable, use default command processing
     */
    virtual unsigned int getNumberOfHardwareLayers(const unsigned int screenID) const = 0;

    /**
     * \brief get resolution of screen
     * \ingroup ServiceAPI
     * \param[in] screenID id of screen
     * \return array of integer with width and height of screen
     * \todo removable, use default command processing
     */
    virtual unsigned int* getScreenResolution(const unsigned int screenID) const = 0;

    /**
     * \brief get list of available screens
     * \ingroup ServiceAPI
     * \param[out] length of returned array
     * \return list of screen ids
     * \todo removable, use default command processing
     */
    virtual unsigned int* getScreenIDs(unsigned int* length) const = 0;

    /**
     * \brief add a notification for an updated scene element
     * \ingroup ServiceAPI
     * \param[in] object pointer to updated scene element, e.g. layer or surface
     * \param[in] mask bitmask indicating which property of scene element was udpated
     */
    virtual void addClientNotification(GraphicalObject* object, t_ilm_notification_mask mask) = 0;

    /**
     * \brief get the list of updated scene elements
     * \ingroup ServiceAPI
     * \return reference to current list of updated scene elements
     */
    virtual NotificationQueue& getClientNotificationQueue() = 0;

    /**
     * \brief get the list of enqueued commands for a client
     * \ingroup ServiceAPI
     * \param[in] clientPid process id of client
     * \return Reference to command list for client
     */
    virtual CommandList& getEnqueuedCommands(unsigned int clientPid) = 0;

    /**
     * \brief get system health state
     * \ingroup ServiceAPI
     * \return system health condition
     */
    virtual HealthCondition getHealth() = 0;
};

#endif /* _COMMANDEXECUTOR_H_ */
