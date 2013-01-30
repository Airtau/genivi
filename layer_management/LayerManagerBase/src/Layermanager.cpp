/***************************************************************************
 *
 * Copyright 2010,2011 BMW Car IT GmbH
 *
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ****************************************************************************/

#include "Layermanager.h"
#include "Log.h"
#include <iomanip>
#include "CommandList.h"
#include "LayerList.h"
#include "ICommand.h"
#include "IHealthMonitor.h"
#include "ICommunicator.h"
#include "IRenderer.h"
#include "ISceneProvider.h"
#include "Scene.h"
#include "Configuration.h"
#include "PluginManager.h"

#include <unistd.h>
#include <pthread.h>
#include <signal.h>

static const char* NO_SENDER_NAME = "unknown";

Layermanager::Layermanager(Configuration& config)
:mConfiguration(config)
{
    m_pScene = new Scene();
    m_pPluginManager = new PluginManager(*this, mConfiguration);
    m_pApplicationReferenceMap = new ApplicationReferenceMap();

    m_pRendererList = new RendererList();
    m_pCommunicatorList = new CommunicatorList();
    m_pSceneProviderList = new SceneProviderList();
    m_pHealthMonitorList = new HealthMonitorList();

    m_pPluginManager->getRendererList(*m_pRendererList);
    m_pPluginManager->getCommunicatorList(*m_pCommunicatorList);
    m_pPluginManager->getSceneProviderList(*m_pSceneProviderList);
    m_pPluginManager->getHealthMonitorList(*m_pHealthMonitorList);

    mHealthState = true;
}

Layermanager::~Layermanager()
{
    if (m_pRendererList)
    {
        delete m_pRendererList;
    }

    if (m_pCommunicatorList)
    {
        delete m_pCommunicatorList;
    }

    if (m_pSceneProviderList)
    {
        delete m_pSceneProviderList;
    }

    if (m_pHealthMonitorList)
    {
        delete m_pHealthMonitorList;
    }

    if (m_pApplicationReferenceMap)
    {
        delete m_pApplicationReferenceMap;
    }

    if (m_pPluginManager)
    {
        delete m_pPluginManager;
    }

    if (m_pScene)
    {
        delete m_pScene;
    }
}

uint Layermanager::getLayerTypeCapabilities(const LayerType layertype) const
{
    uint capabilities = 0;

    // get the first renderer for now TODO
    // change to possibly load multiple renderers

    IRenderer* renderer = *m_pRendererList->begin();
    if (renderer)
    {
        capabilities = renderer->getLayerTypeCapabilities(layertype);
    }

    return capabilities;
}

uint Layermanager::getNumberOfHardwareLayers(const uint screenID) const
{
    uint numberOfHardwareLayers = 0;

    // get the first renderer for now TODO
    // change to possibly load multiple renderers
    IRenderer* renderer = *m_pRendererList->begin();

    if (renderer)
    {
        numberOfHardwareLayers = renderer->getNumberOfHardwareLayers(screenID);
    }

    return numberOfHardwareLayers;
}

uint* Layermanager::getScreenResolution(const uint screenID) const
{
    uint* returnValue = 0;

    // get the first renderer for now TODO
    // change to possibly load multiple renderers
    IRenderer* renderer = *m_pRendererList->begin();

    if (renderer)
    {
        returnValue = renderer->getScreenResolution(screenID);
    }

    return returnValue;
}

uint* Layermanager::getScreenIDs(uint* length) const
{
    uint* returnValue = 0;

    // get the first renderer for now TODO
    // change to possibly load multiple renderers
    IRenderer* renderer = *m_pRendererList->begin();
    if (renderer)
    {
        returnValue = renderer->getScreenIDs(length);
    }

    return returnValue;
}

// This function is used to propagate a signal to the window system
// via the renderer for a redraw because of a rendering property change
void Layermanager::signalRendererRedraw()
{
	IRenderer* renderer = *m_pRendererList->begin();
	if (NULL!=renderer)
		renderer->signalWindowSystemRedraw();
}

void Layermanager::addApplicationReference(t_ilm_client_handle client, IApplicationReference* reference)
{
    if (client && reference)
    {
        m_pidToProcessNameTable[reference->getProcessId()] = reference->getProcessName();
        (*m_pApplicationReferenceMap)[client]=reference;
        unsigned int pid = getSenderPid(client);
        LOG_INFO("LayerManagerService", "Connect from application " << getSenderName(client) << "(" << pid << ")");
    }
}

void Layermanager::removeApplicationReference(t_ilm_client_handle client)
{
    if (client)
    {
        unsigned int pid = getSenderPid(client);
        LOG_INFO("LayerManagerService", "Disconnect from application " << getSenderName(client) << "(" << pid << ")");
        m_pApplicationReferenceMap->erase(client);
        m_pidToProcessNameTable.erase(pid);

        // if pid = 0, the client did not send ServiceConnect.
        // since many client may use this configuration, we must
        // not discard any pending commands here: we could discard
        // commands of currently running applications
        if (0 != pid)
        {
            m_EnqueuedCommands.erase(pid);
        }
    }
}

t_ilm_uint Layermanager::getSenderPid(t_ilm_client_handle client)
{
    t_ilm_uint result = 0;
    IApplicationReference* app = (*m_pApplicationReferenceMap)[client];
    if (app)
    {
        result = app->getProcessId();
    }
    return result;
}

const char*  Layermanager::getSenderName(t_ilm_client_handle client)
{
    const char* result = NO_SENDER_NAME;
    IApplicationReference* app = (*m_pApplicationReferenceMap)[client];
    if (app)
    {
        result = app->getProcessName();
    }
    return result;
}

const char* Layermanager::getSenderName(unsigned int pid)
{
    const char* result = m_pidToProcessNameTable[pid];
    if (!result)
    {
        result = NO_SENDER_NAME;
    }
    return result;
}


void Layermanager::printDebugInformation() const
{
    // print stuff about layerlist
    LOG_INFO("LayerManagerService", "Layer: ID |  X |  Y |  W |  H |Al.| Z \n");

    unsigned int length;
    unsigned int* LayerIDs;

    // loop the layers
    m_pScene->getLayerIDs(&length, &LayerIDs);

    for (unsigned int i = 0; i < length; i++)
    {
        Layer* currentLayer = m_pScene->getLayer(LayerIDs[i]);
        LOG_INFO("LayerManagerService", "      " << std::setw(4) << currentLayer->getID() << "\n");

        LOG_INFO("LayerManagerService", "    Surface:  ID |Al.| Z |  SVP: X |  Y |  W |  H     DVP:  X |  Y |  W |  H \n");

        // loop the surfaces of within each layer
        SurfaceListConstIterator iter = currentLayer->getAllSurfaces().begin();
        SurfaceListConstIterator iterEnd = currentLayer->getAllSurfaces().end();

        for (; iter != iterEnd; ++iter)
        {
            LOG_INFO("LayerManagerService", "            " << std::setw(4) << (*iter)->getID() << std::setw(4) << std::setprecision(3) << (*iter)->opacity << "\n");
        }
    }
}

bool Layermanager::executeCommand(ICommand* commandToBeExecuted)
{
    ExecutionResult status = ExecutionFailed;
    m_pScene->lockScene();
    status = commandToBeExecuted->execute(this);
    m_pScene->unlockScene();

    if (status == ExecutionFailedRedraw || status == ExecutionSuccessRedraw)
    {
    	// either a command that has changed the output was executed, or
    	// commit command was executed which contained commands that requested redraw was executed.
    	signalRendererRedraw();
    }

    unsigned int commandPid = commandToBeExecuted->getSenderPid();
    LOG_INFO("LayerManagerService", "executed " << commandToBeExecuted->getString()
                                     << " from " << getSenderName(commandPid) << "(" << commandPid << ")"
                                     << ((status == ExecutionSuccess || status == ExecutionSuccessRedraw) ? "+" : "-"));

    return (status == ExecutionSuccess || status == ExecutionSuccessRedraw);
}

bool Layermanager::enqueueCommand(ICommand* command)
{
    bool result = false;

    if (command)
    {
        const unsigned int commandPid = command->getSenderPid();
        m_EnqueuedCommands[commandPid].push_back(command);
        result = true;

        LOG_DEBUG("LayerManagerService", "enqueued " << command->getString()
                    << " from " << getSenderName(commandPid) << "(" << commandPid << ")"
                    << (result ? "+" : "-"));
    }

    return result;
}

bool Layermanager::execute(ICommand* commandToBeExecuted)
{
    bool status = false;

    ExecutionType type = commandToBeExecuted->getExecutionType();

    switch (type)
    {
    case ExecuteSynchronous:
        status = executeCommand(commandToBeExecuted);
        delete commandToBeExecuted;
        break;

    case ExecuteAsynchronous:
        status = enqueueCommand(commandToBeExecuted);
        break;

    default:
        // TODO: error
        LOG_ERROR("LayerManagerService", "Command " << commandToBeExecuted->getString() << " was not processed");
        break;
    }

    return status;
}

bool Layermanager::startAllRenderers(const int width, const int height,
        const char *displayName)
{
    bool allStarted = false;

    RendererListIterator iter = m_pRendererList->begin();
    RendererListIterator iterEnd = m_pRendererList->end();
    for (; iter != iterEnd; ++iter)
    {
        IRenderer* renderer = *iter;
        if (renderer)
        {
            allStarted = renderer->start(width, height, displayName);
        }
        if (renderer == NULL || allStarted == false)
        {
          break;   
        }
    }
    if (!allStarted)
    {
        LOG_ERROR("LayerManagerService","Could not start Compositing Controllers");
    }
    return allStarted;
}

bool Layermanager::delegateScene()
{
    bool allStarted = true;

    SceneProviderListIterator iter = m_pSceneProviderList->begin();
    SceneProviderListIterator iterEnd = m_pSceneProviderList->end();

    for (; iter != iterEnd; ++iter)
    {
        ISceneProvider* sceneProvider = *iter;
        if (sceneProvider)
        {
            allStarted &= sceneProvider->delegateScene();
        }
        else
        {
            allStarted = false;
        }
    }
    if (!allStarted)
    {
        LOG_WARNING("LayerManagerService","Could not start delegate Scenes, initial Scene is lost");
    }
    return allStarted;
}

bool Layermanager::startAllCommunicators()
{
    bool allStarted = true;
    
    CommunicatorListIterator communicatorIter = m_pCommunicatorList->begin();
    CommunicatorListIterator communicatorIterEnd = m_pCommunicatorList->end();
    
    for (; communicatorIter != communicatorIterEnd; ++communicatorIter)
    {
        ICommunicator *communicator = *communicatorIter;
        if (communicator)
        {
            allStarted &= communicator->start();
        }
        else
        {
            allStarted = false;
        }
    }
    if (!allStarted)
    {
        LOG_ERROR("LayerManagerService","Could not start Communication Controllers");
    }
    return allStarted;
}

bool Layermanager::startAllHealthMonitors()
{
    bool allStarted = true;
    
    HealthMonitorListIterator iter = m_pHealthMonitorList->begin();
    HealthMonitorListIterator iterEnd = m_pHealthMonitorList->end();
    
    for (; iter != iterEnd; ++iter)
    {
        IHealthMonitor* healthMonitor = *iter;
        if (healthMonitor)
        {
            allStarted &= healthMonitor->start();
        }
        else
        {
            allStarted = false;
        }
    }
    if (!allStarted)
    {
        LOG_ERROR("LayerManagerService","Could not start Health Monitors");
    }
    return allStarted;
}

bool Layermanager::startManagement()
{
    bool result = false;

    const int width = mConfiguration.getDisplayWidth();
    const int height = mConfiguration.getDisplayHeight();
    const char* displayName = mConfiguration.getDisplayName().c_str();
    
    // 1. start renderers, no prerequisites
    // 2. execute scene provider plugins
    // 3. start communication (after scene is ready to use)
    result = startAllRenderers(width, height, displayName)
             && delegateScene()
             && startAllCommunicators()
             && startAllHealthMonitors();

    return result;
}

void Layermanager::stopAllHealthMonitors()
{
    HealthMonitorListIterator iter = m_pHealthMonitorList->begin();
    HealthMonitorListIterator iterEnd = m_pHealthMonitorList->end();
    for (; iter != iterEnd; ++iter)
    {
        IHealthMonitor* healthMonitor = *iter;
        if (healthMonitor)
        {
            healthMonitor->stop();
        }
    }
}

void Layermanager::stopAllRenderers()
{
    RendererListIterator iter = m_pRendererList->begin();
    RendererListIterator iterEnd = m_pRendererList->end();
    for (; iter != iterEnd; ++iter)
    {
        IRenderer *renderer = *iter;
        if (renderer)
        {
            renderer->stop();
        }
    }
}

void Layermanager::stopAllCommunicators()
{
    CommunicatorListIterator iter = m_pCommunicatorList->begin();
    CommunicatorListIterator iterEnd = m_pCommunicatorList->end();
    for (; iter != iterEnd; ++iter)
    {
        ICommunicator *communicator = *iter;
        if (communicator)
        {
            communicator->stop();
        }
    }
}

bool Layermanager::stopManagement()
{
    stopAllHealthMonitors();
    stopAllRenderers();
    stopAllCommunicators();
    return true; // TODO
}

HealthCondition Layermanager::getHealth()
{
    HealthCondition returnValue = HealthRunning;
    PluginList::const_iterator iter = mMonitoredPlugins.begin();
    PluginList::const_iterator iterEnd = mMonitoredPlugins.end();

    while ((iter != iterEnd) && (HealthRunning == returnValue))
    {
        IPlugin* monitoredPlugin = *iter;
        returnValue = monitoredPlugin->pluginGetHealth();
        ++iter;
    }

    return returnValue;
}

