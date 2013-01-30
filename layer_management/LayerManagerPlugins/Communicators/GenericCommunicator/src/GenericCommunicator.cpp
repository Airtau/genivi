/***************************************************************************
 * Copyright 2012 BMW Car IT GmbH
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

#include "GenericCommunicator.h"
#include "ilm_types.h"
#include "LmScreen.h"
#include "Log.h"
#include "Configuration.h"

#include "ICommandExecutor.h"
#include "CommitCommand.h"
#include "LayerCreateCommand.h"
#include "SurfaceCreateCommand.h"
#include "SurfaceGetDimensionCommand.h"
#include "LayerGetDimensionCommand.h"
#include "SurfaceGetOpacityCommand.h"
#include "LayerGetOpacityCommand.h"
#include "SurfaceGetPixelformatCommand.h"
#include "LayerGetVisibilityCommand.h"
#include "SurfaceGetVisibilityCommand.h"
#include "LayerAddSurfaceCommand.h"
#include "LayerRemoveSurfaceCommand.h"
#include "LayerRemoveCommand.h"
#include "SurfaceRemoveCommand.h"
#include "SurfaceGetOrientationCommand.h"
#include "LayerGetOrientationCommand.h"
#include "LayerSetDestinationRectangleCommand.h"
#include "SurfaceSetDestinationRectangleCommand.h"
#include "LayerSetOpacityCommand.h"
#include "SurfaceSetOpacityCommand.h"
#include "LayerSetSourceRectangleCommand.h"
#include "SurfaceSetSourceRectangleCommand.h"
#include "LayerSetOrientationCommand.h"
#include "SurfaceSetOrientationCommand.h"
#include "LayerSetVisibilityCommand.h"
#include "SurfaceSetVisibilityCommand.h"
#include "DebugCommand.h"
#include "ExitCommand.h"
#include "ScreenSetRenderOrderCommand.h"
#include "LayerSetRenderOrderCommand.h"
#include "LayerSetDimensionCommand.h"
#include "SurfaceSetDimensionCommand.h"
#include "LayerSetPositionCommand.h"
#include "SurfaceSetPositionCommand.h"
#include "LayerGetPositionCommand.h"
#include "SurfaceGetPositionCommand.h"
#include "ShaderCreateCommand.h"
#include "ShaderDestroyCommand.h"
#include "SurfaceSetShaderCommand.h"
#include "ShaderSetUniformsCommand.h"
#include "ScreenDumpCommand.h"
#include "LayerDumpCommand.h"
#include "SurfaceDumpCommand.h"
#include "SurfaceSetNativeContentCommand.h"
#include "SurfaceRemoveNativeContentCommand.h"
#include "SurfaceSetKeyboardFocusCommand.h"
#include "SurfaceGetKeyboardFocusCommand.h"
#include "SurfaceUpdateInputEventAcceptance.h"
#include "SurfaceSetChromaKeyCommand.h"
#include "LayerSetChromaKeyCommand.h"
#include "SetOptimizationModeCommand.h"
#include "GetOptimizationModeCommand.h"
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <pthread.h>
#include <signal.h>
#include <vector>

#define DEFAULT_SCREEN 0

const char* RESOURCE_ALREADY_INUSE = "Ressource already in use";
const char* INVALID_ARGUMENT = "Invalid argument";
const char* RESOURCE_NOT_FOUND = "Ressource not found";
const char* NOT_IMPLEMENTED = "Feature not implemented";


GenericCommunicator::GenericCommunicator(ICommandExecutor& executor, Configuration& config)
: ICommunicator(&executor)
, PluginBase(executor, config, Communicator_Api_v1)
, m_running(ILM_FALSE)
{
    MethodTable manager_methods[] =
    {
        { "ServiceConnect",                   &GenericCommunicator::ServiceConnect },
        { "ServiceDisconnect",                &GenericCommunicator::ServiceDisconnect },
        { "Debug",                            &GenericCommunicator::Debug },
        { "ScreenShot",                       &GenericCommunicator::ScreenShot },
        { "ScreenShotOfLayer",                &GenericCommunicator::ScreenShotOfLayer },
        { "ScreenShotOfSurface",              &GenericCommunicator::ScreenShotOfSurface },
        { "GetScreenResolution",              &GenericCommunicator::GetScreenResolution },
        { "GetNumberOfHardwareLayers",        &GenericCommunicator::GetNumberOfHardwareLayers },
        { "GetScreenIDs",                     &GenericCommunicator::GetScreenIDs },
        { "ListAllLayerIDS",                  &GenericCommunicator::ListAllLayerIDS },
        { "ListAllLayerIDsOnScreen",          &GenericCommunicator::ListAllLayerIDsOnScreen },
        { "ListAllSurfaceIDS",                &GenericCommunicator::ListAllSurfaceIDS },
        { "ListSurfaceofLayer",               &GenericCommunicator::ListSurfaceofLayer },
        { "GetPropertiesOfSurface",           &GenericCommunicator::GetPropertiesOfSurface },
        { "GetPropertiesOfLayer",             &GenericCommunicator::GetPropertiesOfLayer },
        { "CreateSurface",                    &GenericCommunicator::CreateSurface },
        { "CreateSurfaceFromId",              &GenericCommunicator::CreateSurfaceFromId },
        { "InitializeSurface",                &GenericCommunicator::InitializeSurface },
        { "InitializeSurfaceFromId",          &GenericCommunicator::InitializeSurfaceFromId },
        { "SetSurfaceNativeContent",          &GenericCommunicator::SetSurfaceNativeContent },
        { "RemoveSurfaceNativeContent",       &GenericCommunicator::RemoveSurfaceNativeContent },
        { "RemoveSurface",                    &GenericCommunicator::RemoveSurface },
        { "CreateLayer",                      &GenericCommunicator::CreateLayer },
        { "CreateLayerFromId",                &GenericCommunicator::CreateLayerFromId },
        { "CreateLayerWithDimension",         &GenericCommunicator::CreateLayerWithDimension },
        { "CreateLayerFromIdWithDimension",   &GenericCommunicator::CreateLayerFromIdWithDimension },
        { "RemoveLayer",                      &GenericCommunicator::RemoveLayer },
        { "AddSurfaceToLayer",                &GenericCommunicator::AddSurfaceToLayer },
        { "RemoveSurfaceFromLayer",           &GenericCommunicator::RemoveSurfaceFromLayer },
        { "SetSurfaceSourceRegion",           &GenericCommunicator::SetSurfaceSourceRegion },
        { "SetLayerSourceRegion",             &GenericCommunicator::SetLayerSourceRegion },
        { "SetSurfaceDestinationRegion",      &GenericCommunicator::SetSurfaceDestinationRegion },
        { "SetSurfacePosition",               &GenericCommunicator::SetSurfacePosition },
        { "GetSurfacePosition",               &GenericCommunicator::GetSurfacePosition },
        { "SetSurfaceDimension",              &GenericCommunicator::SetSurfaceDimension },
        { "SetLayerDestinationRegion",        &GenericCommunicator::SetLayerDestinationRegion },
        { "SetLayerPosition",                 &GenericCommunicator::SetLayerPosition },
        { "GetLayerPosition",                 &GenericCommunicator::GetLayerPosition },
        { "SetLayerDimension",                &GenericCommunicator::SetLayerDimension },
        { "GetLayerDimension",                &GenericCommunicator::GetLayerDimension },
        { "GetSurfaceDimension",              &GenericCommunicator::GetSurfaceDimension },
        { "SetSurfaceOpacity",                &GenericCommunicator::SetSurfaceOpacity },
        { "SetLayerOpacity",                  &GenericCommunicator::SetLayerOpacity },
        { "GetSurfaceOpacity",                &GenericCommunicator::GetSurfaceOpacity },
        { "GetLayerOpacity",                  &GenericCommunicator::GetLayerOpacity },
        { "SetSurfaceOrientation",            &GenericCommunicator::SetSurfaceOrientation },
        { "GetSurfaceOrientation",            &GenericCommunicator::GetSurfaceOrientation },
        { "SetLayerOrientation",              &GenericCommunicator::SetLayerOrientation },
        { "GetLayerOrientation",              &GenericCommunicator::GetLayerOrientation },
        { "GetSurfacePixelformat",            &GenericCommunicator::GetSurfacePixelformat },
        { "SetSurfaceVisibility",             &GenericCommunicator::SetSurfaceVisibility },
        { "SetLayerVisibility",               &GenericCommunicator::SetLayerVisibility },
        { "GetSurfaceVisibility",             &GenericCommunicator::GetSurfaceVisibility },
        { "GetLayerVisibility",               &GenericCommunicator::GetLayerVisibility },
        { "SetRenderOrderOfLayers",           &GenericCommunicator::SetRenderOrderOfLayers },
        { "SetSurfaceRenderOrderWithinLayer", &GenericCommunicator::SetSurfaceRenderOrderWithinLayer },
        { "GetLayerType",                     &GenericCommunicator::GetLayerType },
        { "GetLayertypeCapabilities",         &GenericCommunicator::GetLayertypeCapabilities },
        { "GetLayerCapabilities",             &GenericCommunicator::GetLayerCapabilities },
        { "Exit",                             &GenericCommunicator::Exit },
        { "CommitChanges",                    &GenericCommunicator::CommitChanges },
        { "CreateShader",                     &GenericCommunicator::CreateShader },
        { "DestroyShader",                    &GenericCommunicator::DestroyShader },
        { "SetShader",                        &GenericCommunicator::SetShader },
        { "SetUniforms",                      &GenericCommunicator::SetUniforms },
        { "SetKeyboardFocusOn",               &GenericCommunicator::SetKeyboardFocusOn },
        { "GetKeyboardFocusSurfaceId",        &GenericCommunicator::GetKeyboardFocusSurfaceId },
        { "UpdateInputEventAcceptanceOn",     &GenericCommunicator::UpdateInputEventAcceptanceOn },
        { "SetSurfaceChromaKey",              &GenericCommunicator::SetSurfaceChromaKey },
        { "SetLayerChromaKey",                &GenericCommunicator::SetLayerChromaKey },
        { "LayerAddNotification",             &GenericCommunicator::LayerAddNotification },
        { "SurfaceAddNotification",           &GenericCommunicator::SurfaceAddNotification },
        { "LayerRemoveNotification",          &GenericCommunicator::LayerRemoveNotification },
        { "SurfaceRemoveNotification",        &GenericCommunicator::SurfaceRemoveNotification },
        { "SetOptimizationMode",              &GenericCommunicator::SetOptimizationMode },
        { "GetOptimizationMode",              &GenericCommunicator::GetOptimizationMode },
        { "GetPropertiesOfScreen",            &GenericCommunicator::GetPropertiesOfScreen }
    };

    int entryCount = sizeof(manager_methods) / sizeof(MethodTable);

    for (int index = 0; index < entryCount; ++index)
    {
        MethodTable* method = &manager_methods[index];
        if (method->function)
        {
            m_callBackTable[method->name] = *method;
            LOG_DEBUG("GenericCommunicator", "registered callback for " << method->name);
        }
    }

    memset(&m_ipcModule, 0, sizeof(m_ipcModule));

    mThreadId = pthread_self();
}

bool GenericCommunicator::start()
{
    LOG_DEBUG("GenericCommunicator", "Starting up IpcModules.");

    if (!loadIpcModule(&m_ipcModule))
    {
        LOG_ERROR("GenericCommunicator", "Loading IpcModule failed.");
        return ILM_FALSE;
    }
    LOG_DEBUG("GenericCommunicator", "Loading IpcModule success.");

    if (!m_ipcModule.initServiceMode())
    {
        LOG_ERROR("GenericCommunicator", "Initializing IpcModule failed.");
        return ILM_FALSE;
    }
    LOG_DEBUG("GenericCommunicator", "Initializing IpcModule success.");

    m_running = ILM_TRUE;
    pluginSetHealth(HealthRunning);

    threadCreate();
    threadInit();
    threadStart();

    return ILM_TRUE;
}

void GenericCommunicator::stop()
{
    LOG_INFO("GenericCommunicator","stopping");

    threadStop();
    
    if (m_running)
    {
        m_ipcModule.destroy();
    }
    pluginSetHealth(HealthStopped);
}

void GenericCommunicator::process(int timeout_ms)
{
    t_ilm_message message = m_ipcModule.receive(timeout_ms);
    if (!message)
    {
        return;
    }

    t_ilm_message_type messageType = m_ipcModule.getMessageType(message);
    t_ilm_const_string name = m_ipcModule.getMessageName(message);
    t_ilm_client_handle senderHandle = m_ipcModule.getSenderHandle(message);

    switch(messageType)
    {
    case IpcMessageTypeCommand:
        if (m_callBackTable.end() != m_callBackTable.find(name))
        {
            LOG_DEBUG("GenericCommunicator", "received: " << name << " from "
                       << m_executor->getSenderName(senderHandle)
                       << "(" << m_executor->getSenderPid(senderHandle) << ")");
            CallBackMethod method = m_callBackTable[name].function;
            (this->*method)(message);
        }
        else
        {
            LOG_WARNING("GenericCommunicator", "Received unknown command " << name
                  << " from " << m_executor->getSenderName(senderHandle)
                  << "(pid " << m_executor->getSenderPid(senderHandle) << ")");
        }
        processNotificationQueue();
        break;

    case IpcMessageTypeConnect:
        LOG_DEBUG("GenericCommunicator", "client " << m_executor->getSenderName(senderHandle)
                  << "(pid " << m_executor->getSenderPid(senderHandle) << ") connected");
        break;

    case IpcMessageTypeDisconnect:
        LOG_DEBUG("GenericCommunicator", "client " << m_executor->getSenderName(senderHandle)
                  << "(pid " << m_executor->getSenderPid(senderHandle) << ") disconnected");
        {
            const LayerMap& layers = m_executor->getScene()->getAllLayers();
            LayerMapConstIterator layerIter =  layers.begin();
            LayerMapConstIterator layerIterEnd = layers.end();
            for (; layerIter != layerIterEnd; ++layerIter)
            {
                Layer* layer = layerIter->second;
                layer->removeNotification(senderHandle);
            }

            const SurfaceMap& surfaces = m_executor->getScene()->getAllSurfaces();
            SurfaceMapConstIterator surfaceIter =  surfaces.begin();
            SurfaceMapConstIterator surfaceIterEnd = surfaces.end();
            for (; surfaceIter != surfaceIterEnd; ++surfaceIter)
            {
                Surface* surface = surfaceIter->second;
                surface->removeNotification(senderHandle);
            }
        }
        break;

    case IpcMessageTypeError:
        LOG_DEBUG("GenericCommunicator", "Received error message " << name << " from "
                  << m_executor->getSenderName(senderHandle)
                  << "(pid " << m_executor->getSenderPid(senderHandle) << ")");
        break;

    case IpcMessageTypeShutdown:
    case IpcMessageTypeNone:
        break;

    default:
        LOG_DEBUG("GenericCommunicator", "Received unknown data from "
                  << m_executor->getSenderName(senderHandle)
                  << "(pid " << m_executor->getSenderPid(senderHandle) << "), Message type: " << messageType);
        break;
    }
    m_ipcModule.destroyMessage(message);
}

void GenericCommunicator::setdebug(bool onoff)
{
    (void)onoff; // TODO: remove, only prevents warning
}

t_ilm_bool GenericCommunicator::threadMainLoop()
{
    process(-1);
    return ILM_TRUE;
}

t_ilm_const_string GenericCommunicator::pluginGetName() const
{
    return "GenericCommunicator";
}

void GenericCommunicator::ServiceConnect(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);

    unsigned int processId = 0;
    char processName[1024];
    m_ipcModule.getUint(message, &processId);
    m_ipcModule.getString(message, processName);

    m_executor->addApplicationReference(clientHandle, new IApplicationReference(processName, processId));

    LOG_DEBUG("GenericCommunicator", "ServiceConnect called from "
              << m_executor->getSenderName(clientHandle)
              << "(" << m_executor->getSenderPid(clientHandle) << ")");

    response = m_ipcModule.createResponse(message);
    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::ServiceDisconnect(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);

    LOG_DEBUG("GenericCommunicator", "ServiceDisconnect called from "
              << m_executor->getSenderName(clientHandle)
              << "(" << m_executor->getSenderPid(clientHandle) << ")");

    m_executor->removeApplicationReference(clientHandle);

    response = m_ipcModule.createResponse(message);
    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::Debug(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    t_ilm_bool param = ILM_FALSE;
    m_ipcModule.getBool(message, &param);

    t_ilm_bool status = m_executor->execute(new DebugCommand(clientPid, param));
    if (status)
    {
        response = m_ipcModule.createResponse(message);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, INVALID_ARGUMENT);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::GetScreenResolution(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    uint screenid = 0;
    m_ipcModule.getUint(message, &screenid);
    uint* resolution = m_executor->getScreenResolution(screenid);

    response = m_ipcModule.createResponse(message);
    m_ipcModule.appendUint(response, resolution[0]);
    m_ipcModule.appendUint(response, resolution[1]);
    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::GetNumberOfHardwareLayers(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    uint screenid = 0;
    m_ipcModule.getUint(message, &screenid);
    uint numberOfHardwareLayers = m_executor->getNumberOfHardwareLayers(screenid);
    response = m_ipcModule.createResponse(message);
    m_ipcModule.appendUint(response, numberOfHardwareLayers);
    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::GetScreenIDs(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    uint length = 0;
    uint* IDs = m_executor->getScreenIDs(&length);
    response = m_ipcModule.createResponse(message);
    m_ipcModule.appendUintArray(response, IDs, length);
    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::ScreenShot(t_ilm_message message)
{
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    t_ilm_message response;
    uint screenid = 0;
    char filename[1024];

    m_ipcModule.getUint(message, &screenid);
    m_ipcModule.getString(message, filename);

    t_ilm_bool status = m_executor->execute(new ScreenDumpCommand(clientPid, filename, screenid));
    if (status)
    {
        response = m_ipcModule.createResponse(message);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, INVALID_ARGUMENT);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::ScreenShotOfLayer(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    char filename[1024];
    m_ipcModule.getString(message, filename);
    uint layerid = 0;
    m_ipcModule.getUint(message, &layerid);

    t_ilm_bool status = m_executor->execute(new LayerDumpCommand(clientPid, filename, layerid));
    if (status)
    {
        response = m_ipcModule.createResponse(message);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_NOT_FOUND);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::ScreenShotOfSurface(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    char filename[1024];
    m_ipcModule.getString(message, filename);
    uint id = 0;
    m_ipcModule.getUint(message, &id);
    t_ilm_bool status = m_executor->execute(new SurfaceDumpCommand(clientPid, filename, id));
    if (status)
    {
        response = m_ipcModule.createResponse(message);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_NOT_FOUND);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::ListAllLayerIDS(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    uint* array = NULL;
    uint length = 0;
    m_executor->getScene()->lockScene();
    m_executor->getScene()->getLayerIDs(&length, &array);
    m_executor->getScene()->unlockScene();
    response = m_ipcModule.createResponse(message);
    m_ipcModule.appendUintArray(response, array, length);
    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::ListAllLayerIDsOnScreen(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    uint screenID = 0;
    m_ipcModule.getUint(message, &screenID);

    uint* array = NULL;
    uint length = 0;
    m_executor->getScene()->lockScene();
    t_ilm_bool status = m_executor->getScene()->getLayerIDsOfScreen(screenID, &length, &array);
    m_executor->getScene()->unlockScene();
    if (status)
    {
        response = m_ipcModule.createResponse(message);
        m_ipcModule.appendUintArray(response, array, length);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, INVALID_ARGUMENT);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::ListAllSurfaceIDS(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    uint* array = NULL;
    uint length = 0;
    m_executor->getScene()->lockScene();
    m_executor->getScene()->getSurfaceIDs(&length, &array);
    m_executor->getScene()->unlockScene();
    response = m_ipcModule.createResponse(message);
    m_ipcModule.appendUintArray(response, array, length);
    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::ListSurfaceofLayer(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    uint id = 0;
    m_ipcModule.getUint(message, &id);
    m_executor->getScene()->lockScene();
    Layer* layer = m_executor->getScene()->getLayer(id);
    if (layer != NULL)
    {
        std::list<Surface*> surfaces = layer->getAllSurfaces();

        uint length = surfaces.size();
        uint* array = new uint[length];
        uint arrayPos = 0;

        for (std::list<Surface*>::const_iterator it = surfaces.begin(); it != surfaces.end(); ++it)
        {
            Surface* s = *it;
            array[arrayPos] = s->getID();
            ++arrayPos;
        }
        m_executor->getScene()->unlockScene();

        response = m_ipcModule.createResponse(message);
        m_ipcModule.appendUintArray(response, array, length);
    }
    else
    {
        m_executor->getScene()->unlockScene();
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_NOT_FOUND);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::GetPropertiesOfSurface(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    uint id = 0;
    m_ipcModule.getUint(message, &id);

    Surface* surface = m_executor->getScene()->getSurface(id);
    if (surface != NULL)
    {
        Rectangle dest = surface->getDestinationRegion();
        Rectangle src = surface->getSourceRegion();
        OrientationType orientation = surface->getOrientation();
        unsigned char chromaKeyRed = 0;
        unsigned char chromaKeyGreen = 0;
        unsigned char chromaKeyBlue = 0;
        surface->getChromaKey(chromaKeyRed, chromaKeyGreen, chromaKeyBlue);

        response = m_ipcModule.createResponse(message);
        m_ipcModule.appendDouble(response, surface->getOpacity());
        m_ipcModule.appendUint(response, src.x);
        m_ipcModule.appendUint(response, src.y);
        m_ipcModule.appendUint(response, src.width);
        m_ipcModule.appendUint(response, src.height);
        m_ipcModule.appendUint(response, surface->OriginalSourceWidth);
        m_ipcModule.appendUint(response, surface->OriginalSourceHeight);
        m_ipcModule.appendUint(response, dest.x);
        m_ipcModule.appendUint(response, dest.y);
        m_ipcModule.appendUint(response, dest.width);
        m_ipcModule.appendUint(response, dest.height);
        m_ipcModule.appendUint(response, orientation);
        m_ipcModule.appendBool(response, surface->getVisibility());
        m_ipcModule.appendUint(response, surface->frameCounter);
        m_ipcModule.appendUint(response, surface->drawCounter);
        m_ipcModule.appendUint(response, surface->updateCounter);
        m_ipcModule.appendUint(response, surface->getPixelFormat());
        m_ipcModule.appendUint(response, surface->getNativeContent());
        m_ipcModule.appendUint(response, surface->getInputEventAcceptanceOnDevices());
        m_ipcModule.appendBool(response, surface->getChromaKeyEnabled());
        m_ipcModule.appendUint(response, chromaKeyRed);
        m_ipcModule.appendUint(response, chromaKeyGreen);
        m_ipcModule.appendUint(response, chromaKeyBlue);
        m_ipcModule.appendInt(response, surface->getCreatorPid());
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_NOT_FOUND);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::GetPropertiesOfLayer(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    uint id = 0;
    m_ipcModule.getUint(message, &id);

    Layer* layer = m_executor->getScene()->getLayer(id);
    if (layer != NULL)
    {
        Rectangle dest = layer->getDestinationRegion();
        Rectangle src = layer->getSourceRegion();
        OrientationType orientation = layer->getOrientation();
        unsigned char chromaKeyRed = 0;
        unsigned char chromaKeyGreen = 0;
        unsigned char chromaKeyBlue = 0;
        layer->getChromaKey(chromaKeyRed, chromaKeyGreen, chromaKeyBlue);

        response = m_ipcModule.createResponse(message);
        m_ipcModule.appendDouble(response, layer->getOpacity());
        m_ipcModule.appendUint(response, src.x);
        m_ipcModule.appendUint(response, src.y);
        m_ipcModule.appendUint(response, src.width);
        m_ipcModule.appendUint(response, src.height);
        m_ipcModule.appendUint(response, layer->OriginalSourceWidth);
        m_ipcModule.appendUint(response, layer->OriginalSourceHeight);
        m_ipcModule.appendUint(response, dest.x);
        m_ipcModule.appendUint(response, dest.y);
        m_ipcModule.appendUint(response, dest.width);
        m_ipcModule.appendUint(response, dest.height);
        m_ipcModule.appendUint(response, orientation);
        m_ipcModule.appendBool(response, layer->getVisibility());
        m_ipcModule.appendUint(response, layer->getLayerType());
        m_ipcModule.appendBool(response, layer->getChromaKeyEnabled());
        m_ipcModule.appendUint(response, chromaKeyRed);
        m_ipcModule.appendUint(response, chromaKeyGreen);
        m_ipcModule.appendUint(response, chromaKeyBlue);
        m_ipcModule.appendInt(response, layer->getCreatorPid());
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_NOT_FOUND);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::CreateSurface(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    uint handle = 0;
    uint width = 0;
    uint height = 0;
    uint pixelformat = 0;
    PixelFormat pf = PIXELFORMAT_UNKNOWN;
    uint id = GraphicalObject::INVALID_ID;

    m_ipcModule.getUint(message, &handle);
    m_ipcModule.getUint(message, &width);
    m_ipcModule.getUint(message, &height);
    m_ipcModule.getUint(message, &pixelformat);

    pf = (PixelFormat) pixelformat;


    // First of all create the surface
    t_ilm_bool status = m_executor->execute(new SurfaceCreateCommand(clientPid, &id));

    // after that apply the native content
    status &= m_executor->execute(new SurfaceSetNativeContentCommand(clientPid, id, handle, pf, width, height));

    if (status)
    {
        response = m_ipcModule.createResponse(message);
        m_ipcModule.appendUint(response, id);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_ALREADY_INUSE);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::CreateSurfaceFromId(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    uint handle = 0;
    uint width = 0;
    uint height = 0;
    uint pixelformat = 0;
    PixelFormat pf = PIXELFORMAT_UNKNOWN;
    uint id = 0;

    m_ipcModule.getUint(message, &handle);
    m_ipcModule.getUint(message, &width);
    m_ipcModule.getUint(message, &height);
    m_ipcModule.getUint(message, &pixelformat);
    m_ipcModule.getUint(message, &id);

    pf = (PixelFormat) pixelformat;

    // First of all create the surface
    t_ilm_bool status = m_executor->execute(new SurfaceCreateCommand(clientPid, &id));

    // after that apply the native content
    status &= m_executor->execute(new SurfaceSetNativeContentCommand(clientPid, id, handle, pf, width, height));

    if (status)
    {
        response = m_ipcModule.createResponse(message);
        m_ipcModule.appendUint(response, id);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_ALREADY_INUSE);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::InitializeSurface(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    uint id = GraphicalObject::INVALID_ID;

    t_ilm_bool status = m_executor->execute(new SurfaceCreateCommand(clientPid, &id));
    if (status)
    {
        response = m_ipcModule.createResponse(message);
        m_ipcModule.appendUint(response, id);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_ALREADY_INUSE);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::InitializeSurfaceFromId(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    uint id = 0;
    m_ipcModule.getUint(message, &id);
    t_ilm_bool status = m_executor->execute(new SurfaceCreateCommand(clientPid, &id));
    if (status)
    {
        response = m_ipcModule.createResponse(message);
        m_ipcModule.appendUint(response, id);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_ALREADY_INUSE);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::SetSurfaceNativeContent(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    uint id = 0;
    uint handle = 0;
    uint width = 0;
    uint height = 0;
    uint pixelformat = 0;
    PixelFormat pf = PIXELFORMAT_UNKNOWN;

    m_ipcModule.getUint(message, &id);
    m_ipcModule.getUint(message, &handle);
    m_ipcModule.getUint(message, &width);
    m_ipcModule.getUint(message, &height);
    m_ipcModule.getUint(message, &pixelformat);

    pf = (PixelFormat) pixelformat;

    t_ilm_bool status = m_executor->execute(new SurfaceSetNativeContentCommand(clientPid, id, handle, pf, width, height));
    if (status)
    {
        response = m_ipcModule.createResponse(message);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_ALREADY_INUSE);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::RemoveSurfaceNativeContent(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    uint id = 0;
    m_ipcModule.getUint(message, &id);

    t_ilm_bool status = m_executor->execute(new SurfaceRemoveNativeContentCommand(clientPid, id));
    if (status)
    {
        response = m_ipcModule.createResponse(message);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_ALREADY_INUSE);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::RemoveSurface(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    uint param = 0;
    m_ipcModule.getUint(message, &param);
    t_ilm_bool status = m_executor->execute(new SurfaceRemoveCommand(clientPid, param));
    if (status)
    {
        response = m_ipcModule.createResponse(message);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_ALREADY_INUSE);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::CreateLayer(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    uint id = GraphicalObject::INVALID_ID;
    // use resolution of default screen as default width and height of layers
    uint* resolution = m_executor->getScreenResolution(DEFAULT_SCREEN);
    t_ilm_bool status = m_executor->execute(new LayerCreateCommand(clientPid, resolution[0], resolution[1], &id));
    if (status)
    {
        response = m_ipcModule.createResponse(message);
        m_ipcModule.appendUint(response, id);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_ALREADY_INUSE);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::CreateLayerFromId(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    uint id = GraphicalObject::INVALID_ID;

    m_ipcModule.getUint(message, &id);
    // use resolution of default screen as default width and height of layers
    uint* resolution = m_executor->getScreenResolution(DEFAULT_SCREEN);
    t_ilm_bool status = m_executor->execute(new LayerCreateCommand(clientPid, resolution[0], resolution[1], &id));
    if (status)
    {
        response = m_ipcModule.createResponse(message);
        m_ipcModule.appendUint(response, id);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_ALREADY_INUSE);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}


void GenericCommunicator::CreateLayerWithDimension(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    uint width = 0;
    m_ipcModule.getUint(message, &width);
    uint height = 0;
    m_ipcModule.getUint(message, &height);

    uint id = GraphicalObject::INVALID_ID;
    t_ilm_bool status = m_executor->execute(new LayerCreateCommand(clientPid, width, height, &id));
    if (status)
    {
        response = m_ipcModule.createResponse(message);
        m_ipcModule.appendUint(response, id);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_NOT_FOUND);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::CreateLayerFromIdWithDimension(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    uint id = GraphicalObject::INVALID_ID;
    uint width = 0;
    uint height = 0;
    t_ilm_bool status = ILM_FALSE;

    m_ipcModule.getUint(message, &id);
    m_ipcModule.getUint(message, &width);
    m_ipcModule.getUint(message, &height);

    status = m_executor->execute(new LayerCreateCommand(clientPid, width, height, &id));
    if (status)
    {
        response = m_ipcModule.createResponse(message);
        m_ipcModule.appendUint(response, id);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_ALREADY_INUSE);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::RemoveLayer(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    uint param = 0;
    m_ipcModule.getUint(message, &param);
    t_ilm_bool status = m_executor->execute(new LayerRemoveCommand(clientPid, param));
    if (status)
    {
        response = m_ipcModule.createResponse(message);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_NOT_FOUND);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::AddSurfaceToLayer(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    uint surfaceid = 0;
    uint layer = 0;

    m_ipcModule.getUint(message, &surfaceid);
    m_ipcModule.getUint(message, &layer);

    t_ilm_bool status = m_executor->execute(new LayerAddSurfaceCommand(clientPid, layer, surfaceid));
    if (status)
    {
        response = m_ipcModule.createResponse(message);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_ALREADY_INUSE);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::RemoveSurfaceFromLayer(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    uint surfaceid = 0;
    uint layerid = 0;
    m_ipcModule.getUint(message, &surfaceid);
    m_ipcModule.getUint(message, &layerid);

    t_ilm_bool status = m_executor->execute(new LayerRemoveSurfaceCommand(clientPid, layerid, surfaceid));
    if (status)
    {
        response = m_ipcModule.createResponse(message);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_NOT_FOUND);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::SetSurfaceSourceRegion(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    uint id = 0;
    uint x = 0;
    uint y = 0;
    uint width = 0;
    uint height = 0;

    m_ipcModule.getUint(message, &id);
    m_ipcModule.getUint(message, &x);
    m_ipcModule.getUint(message, &y);
    m_ipcModule.getUint(message, &width);
    m_ipcModule.getUint(message, &height);

    t_ilm_bool status = m_executor->execute(new SurfaceSetSourceRectangleCommand(clientPid, id, x, y, width, height));
    if (status)
    {
        response = m_ipcModule.createResponse(message);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_NOT_FOUND);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::SetLayerSourceRegion(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    uint id = 0;
    uint x = 0;
    uint y = 0;
    uint width = 0;
    uint height = 0;

    m_ipcModule.getUint(message, &id);
    m_ipcModule.getUint(message, &x);
    m_ipcModule.getUint(message, &y);
    m_ipcModule.getUint(message, &width);
    m_ipcModule.getUint(message, &height);

    t_ilm_bool status = m_executor->execute(new LayerSetSourceRectangleCommand(clientPid, id, x, y, width, height));
    if (status)
    {
        response = m_ipcModule.createResponse(message);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_NOT_FOUND);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::SetSurfaceDestinationRegion(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    uint id = 0;
    uint x = 0;
    uint y = 0;
    uint width = 0;
    uint height = 0;

    m_ipcModule.getUint(message, &id);
    m_ipcModule.getUint(message, &x);
    m_ipcModule.getUint(message, &y);
    m_ipcModule.getUint(message, &width);
    m_ipcModule.getUint(message, &height);

    t_ilm_bool status = m_executor->execute(new SurfaceSetDestinationRectangleCommand(clientPid, id, x, y, width, height));
    if (status)
    {
        response = m_ipcModule.createResponse(message);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_NOT_FOUND);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::SetSurfacePosition(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    uint id = 0;
    uint x = 0;
    uint y = 0;

    m_ipcModule.getUint(message, &id);
    m_ipcModule.getUint(message, &x);
    m_ipcModule.getUint(message, &y);

    t_ilm_bool status = m_executor->execute(new SurfaceSetPositionCommand(clientPid, id, x, y));
    if (status)
    {
        response = m_ipcModule.createResponse(message);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_NOT_FOUND);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::GetSurfacePosition(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    uint id = 0;
    uint x = 0;
    uint y = 0;

    m_ipcModule.getUint(message, &id);

    t_ilm_bool status = m_executor->execute(new SurfaceGetPositionCommand(clientPid, id, &x, &y));
    if (status)
    {
        response = m_ipcModule.createResponse(message);
        m_ipcModule.appendUint(response, x);
        m_ipcModule.appendUint(response, y);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_NOT_FOUND);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::SetSurfaceDimension(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    uint id = 0;
    uint width = 0;
    uint height = 0;

    m_ipcModule.getUint(message, &id);
    m_ipcModule.getUint(message, &width);
    m_ipcModule.getUint(message, &height);

    t_ilm_bool status = m_executor->execute(new SurfaceSetDimensionCommand(clientPid, id, width, height));
    if (status)
    {
        response = m_ipcModule.createResponse(message);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_NOT_FOUND);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::SetLayerDestinationRegion(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    uint id = 0;
    uint x = 0;
    uint y = 0;
    uint width = 0;
    uint height = 0;

    m_ipcModule.getUint(message, &id);
    m_ipcModule.getUint(message, &x);
    m_ipcModule.getUint(message, &y);
    m_ipcModule.getUint(message, &width);
    m_ipcModule.getUint(message, &height);

    t_ilm_bool status = m_executor->execute(new LayerSetDestinationRectangleCommand(clientPid, id, x, y, width, height));
    if (status)
    {
        response = m_ipcModule.createResponse(message);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_NOT_FOUND);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::SetLayerPosition(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    uint id = 0;
    uint x = 0;
    uint y = 0;

    m_ipcModule.getUint(message, &id);
    m_ipcModule.getUint(message, &x);
    m_ipcModule.getUint(message, &y);

    t_ilm_bool status = m_executor->execute(new LayerSetPositionCommand(clientPid, id, x, y));
    if (status)
    {
        response = m_ipcModule.createResponse(message);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_NOT_FOUND);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::GetLayerPosition(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    uint id = 0;
    uint x = 0;
    uint y = 0;

    m_ipcModule.getUint(message, &id);

    t_ilm_bool status = m_executor->execute(new LayerGetPositionCommand(clientPid, id, &x, &y));
    if (status)
    {
        response = m_ipcModule.createResponse(message);
        m_ipcModule.appendUint(response, x);
        m_ipcModule.appendUint(response, y);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_NOT_FOUND);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::SetLayerDimension(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    uint id = 0;
    uint width = 0;
    uint height = 0;

    m_ipcModule.getUint(message, &id);
    m_ipcModule.getUint(message, &width);
    m_ipcModule.getUint(message, &height);

    t_ilm_bool status = m_executor->execute(new LayerSetDimensionCommand(clientPid, id, width, height));
    if (status)
    {
        response = m_ipcModule.createResponse(message);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_NOT_FOUND);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::GetLayerDimension(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    uint id = 0;
    uint width = 0;
    uint height = 0;

    m_ipcModule.getUint(message, &id);

    t_ilm_bool status = m_executor->execute(new LayerGetDimensionCommand(clientPid, id, &width, &height));
    if (status)
    {
        response = m_ipcModule.createResponse(message);
        m_ipcModule.appendUint(response, width);
        m_ipcModule.appendUint(response, height);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_NOT_FOUND);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::GetSurfaceDimension(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    uint id = 0;
    uint width = 0;
    uint height = 0;

    m_ipcModule.getUint(message, &id);

    t_ilm_bool status = m_executor->execute(new SurfaceGetDimensionCommand(clientPid, id, &width, &height));
    if (status)
    {
        response = m_ipcModule.createResponse(message);
        m_ipcModule.appendUint(response, width);
        m_ipcModule.appendUint(response, height);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_NOT_FOUND);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::SetSurfaceOpacity(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    uint id = 0;
    double param = 0.0;

    m_ipcModule.getUint(message, &id);
    m_ipcModule.getDouble(message, &param);

    t_ilm_bool status = m_executor->execute(new SurfaceSetOpacityCommand(clientPid, id, param));
    if (status)
    {
        response = m_ipcModule.createResponse(message);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_NOT_FOUND);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::SetLayerOpacity(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    uint id = 0;
    double param = 0.0;

    m_ipcModule.getUint(message, &id);
    m_ipcModule.getDouble(message, &param);

    t_ilm_bool status = m_executor->execute(new LayerSetOpacityCommand(clientPid, id, param));
    if (status)
    {
        response = m_ipcModule.createResponse(message);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_NOT_FOUND);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::GetSurfaceOpacity(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    uint id = 0;
    double param = 0.0;

    m_ipcModule.getUint(message, &id);

    t_ilm_bool status = m_executor->execute(new SurfaceGetOpacityCommand(clientPid, id, &param));
    if (status)
    {
        response = m_ipcModule.createResponse(message);
        m_ipcModule.appendDouble(response, param);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_NOT_FOUND);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::GetLayerOpacity(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    uint id = 0;
    double param = 0.0;

    m_ipcModule.getUint(message, &id);

    t_ilm_bool status = m_executor->execute(new LayerGetOpacityCommand(clientPid, id, &param));
    if (status)
    {
        response = m_ipcModule.createResponse(message);
        m_ipcModule.appendDouble(response, param);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_NOT_FOUND);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::SetSurfaceOrientation(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    uint id = 0;
    uint param = 0;
    OrientationType o = Zero;

    m_ipcModule.getUint(message, &id);
    m_ipcModule.getUint(message, &param);

    o = (OrientationType) param;

    t_ilm_bool status = m_executor->execute(new SurfaceSetOrientationCommand(clientPid, id, o));
    if (status)
    {
        response = m_ipcModule.createResponse(message);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_NOT_FOUND);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::GetSurfaceOrientation(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    uint id = 0;
    OrientationType o;

    m_ipcModule.getUint(message, &id);

    t_ilm_bool status = m_executor->execute(new SurfaceGetOrientationCommand(clientPid, id, &o));
    if (status)
    {
        response = m_ipcModule.createResponse(message);
        m_ipcModule.appendUint(response, o);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_NOT_FOUND);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::SetLayerOrientation(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    uint id = 0;
    uint param = 0;
    OrientationType o = Zero;

    m_ipcModule.getUint(message, &id);
    m_ipcModule.getUint(message, &param);

    o = (OrientationType) param;

    t_ilm_bool status = m_executor->execute(new LayerSetOrientationCommand(clientPid, id, o));
    if (status)
    {
        response = m_ipcModule.createResponse(message);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_NOT_FOUND);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::GetLayerOrientation(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    uint id = 0;
    OrientationType o;

    m_ipcModule.getUint(message, &id);

    t_ilm_bool status = m_executor->execute(new LayerGetOrientationCommand(clientPid, id, &o));
    if (status)
    {
        response = m_ipcModule.createResponse(message);
        m_ipcModule.appendUint(response, o);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_NOT_FOUND);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::GetSurfacePixelformat(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    uint id = 0;
    PixelFormat pixelFormat;

    m_ipcModule.getUint(message, &id);

    t_ilm_bool status = m_executor->execute(new SurfaceGetPixelformatCommand(clientPid, id, &pixelFormat));
    if (status)
    {
        response = m_ipcModule.createResponse(message);
        m_ipcModule.appendUint(response, pixelFormat);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_NOT_FOUND);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::SetSurfaceVisibility(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    uint surfaceid = 0;
    t_ilm_bool newVis = ILM_FALSE;

    m_ipcModule.getUint(message, &surfaceid);
    m_ipcModule.getBool(message, &newVis);

    t_ilm_bool status = m_executor->execute(new SurfaceSetVisibilityCommand(clientPid, surfaceid, newVis));
    if (status)
    {
        response = m_ipcModule.createResponse(message);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_NOT_FOUND);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::SetLayerVisibility(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    uint layerid = 0;
    t_ilm_bool myparam = ILM_FALSE;

    m_ipcModule.getUint(message, &layerid);
    m_ipcModule.getBool(message, &myparam);

    t_ilm_bool status = m_executor->execute(new LayerSetVisibilityCommand(clientPid, layerid, myparam));
    if (status)
    {
        response = m_ipcModule.createResponse(message);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_NOT_FOUND);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::GetSurfaceVisibility(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    uint id = 0;
    bool visibility;

    m_ipcModule.getUint(message, &id);

    t_ilm_bool status = m_executor->execute(new SurfaceGetVisibilityCommand(clientPid, id, &visibility));
    if (status)
    {
        response = m_ipcModule.createResponse(message);
        m_ipcModule.appendBool(response, visibility);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_NOT_FOUND);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::GetLayerVisibility(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    uint id = 0;
    bool visibility;

    m_ipcModule.getUint(message, &id);

    t_ilm_bool status = m_executor->execute(new LayerGetVisibilityCommand(clientPid, id, &visibility));
    if (status)
    {
        response = m_ipcModule.createResponse(message);
        m_ipcModule.appendBool(response, visibility);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_NOT_FOUND);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::SetRenderOrderOfLayers(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    uint* array = NULL;
    int length = 0;
    uint screenID = 0;

    m_ipcModule.getUintArray(message, &array, &length);
    m_ipcModule.getUint(message, &screenID);

    t_ilm_bool status = m_executor->execute(new ScreenSetRenderOrderCommand(clientPid, screenID, array, length));
    if (status)
    {
        response = m_ipcModule.createResponse(message);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_NOT_FOUND);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::SetSurfaceRenderOrderWithinLayer(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    uint* array = NULL;
    int length = 0;
    uint layerid = 0;

    m_ipcModule.getUint(message, &layerid);
    m_ipcModule.getUintArray(message, &array, &length);

    t_ilm_bool status = m_executor->execute(new LayerSetRenderOrderCommand(clientPid, layerid, array, length));
    if (status)
    {
        response = m_ipcModule.createResponse(message);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_NOT_FOUND);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::GetLayerType(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    uint id = 0;
    m_ipcModule.getUint(message, &id);

    Layer* layer = m_executor->getScene()->getLayer(id);
    if (layer)
    {
        response = m_ipcModule.createResponse(message);
        m_ipcModule.appendUint(response, layer->getLayerType());
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_NOT_FOUND);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::GetLayertypeCapabilities(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    uint id = 0;
    LayerType type = Unknown;

    m_ipcModule.getUint(message, &id);

    type = (LayerType) id;

    uint capabilities = m_executor->getLayerTypeCapabilities(type);
    response = m_ipcModule.createResponse(message);
    m_ipcModule.appendUint(response, capabilities);
    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::GetLayerCapabilities(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    uint id = 0;
    m_ipcModule.getUint(message, &id);

    Layer* layer = m_executor->getScene()->getLayer(id);
    if (layer)
    {
        response = m_ipcModule.createResponse(message);
        m_ipcModule.appendUint(response, layer->getCapabilities());
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_NOT_FOUND);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::FadeIn(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    response = m_ipcModule.createErrorResponse(message);
    m_ipcModule.appendString(response, NOT_IMPLEMENTED);
    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::SynchronizedFade(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    response = m_ipcModule.createErrorResponse(message);
    m_ipcModule.appendString(response, NOT_IMPLEMENTED);
    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::FadeOut(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    response = m_ipcModule.createErrorResponse(message);
    m_ipcModule.appendString(response, NOT_IMPLEMENTED);
    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::Exit(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    t_ilm_bool status = m_executor->execute(new ExitCommand(clientPid));
    if (status)
    {
        response = m_ipcModule.createResponse(message);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, INVALID_ARGUMENT);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::CommitChanges(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    t_ilm_bool status = m_executor->execute(new CommitCommand(clientPid));
    if (status)
    {
        response = m_ipcModule.createResponse(message);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, INVALID_ARGUMENT);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::CreateShader(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    char vertname[1024];
    char fragname[1024];
    uint id = 0;

    m_ipcModule.getString(message, vertname);
    m_ipcModule.getString(message, fragname);

    t_ilm_bool status = m_executor->execute(new ShaderCreateCommand(clientPid, vertname, fragname, &id));
    if (status)
    {
        response = m_ipcModule.createResponse(message);
        m_ipcModule.appendUint(response, id);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_NOT_FOUND);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::DestroyShader(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    uint shaderid = 0;

    m_ipcModule.getUint(message, &shaderid);

    t_ilm_bool status = m_executor->execute(new ShaderDestroyCommand(clientPid, shaderid));
    if (status)
    {
        response = m_ipcModule.createResponse(message);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_NOT_FOUND);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::SetShader(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    uint surfaceId = 0;
    uint shaderid = 0;

    m_ipcModule.getUint(message, &surfaceId);
    m_ipcModule.getUint(message, &shaderid);

    t_ilm_bool status = m_executor->execute(new SurfaceSetShaderCommand(clientPid, surfaceId, shaderid));
    if (status)
    {
        response = m_ipcModule.createResponse(message);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_NOT_FOUND);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::SetUniforms(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    uint id = 0;
    std::vector<string> uniforms;

    m_ipcModule.getUint(message, &id);

    // TODO
    //m_ipcModule.getStringArray(&uniforms);

    t_ilm_bool status = m_executor->execute(new ShaderSetUniformsCommand(clientPid, id, uniforms));
    if (status)
    {
        response = m_ipcModule.createResponse(message);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_NOT_FOUND);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::SetKeyboardFocusOn(t_ilm_message message)
{
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    t_ilm_message response;
    uint surfaceId = 0;

    m_ipcModule.getUint(message, &surfaceId);

    t_ilm_bool status = m_executor->execute(new SurfaceSetKeyboardFocusCommand(clientPid, surfaceId));
    if (status)
    {
        response = m_ipcModule.createResponse(message);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_NOT_FOUND);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}


void GenericCommunicator::GetKeyboardFocusSurfaceId(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    uint surfaceId;

    t_ilm_bool status = m_executor->execute(new SurfaceGetKeyboardFocusCommand(clientPid, &surfaceId));
    if (status)
    {
        response = m_ipcModule.createResponse(message);
        m_ipcModule.appendUint(response, surfaceId);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_NOT_FOUND);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}


void GenericCommunicator::UpdateInputEventAcceptanceOn(t_ilm_message message)
{
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    t_ilm_message response;
    uint surfaceId = 0;
    uint udevices = 0;
    InputDevice devices;
    t_ilm_bool accept;

    m_ipcModule.getUint(message, &surfaceId);
    m_ipcModule.getUint(message, &udevices);
    m_ipcModule.getBool(message, &accept);

    devices = (InputDevice) udevices;

    t_ilm_bool status = m_executor->execute(new SurfaceUpdateInputEventAcceptance(clientPid, surfaceId, devices, accept));
    if (status)
    {
        response = m_ipcModule.createResponse(message);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_NOT_FOUND);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::SetSurfaceChromaKey(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    uint* array = NULL;
    int length = 0;
    uint surfaceid = 0;

    m_ipcModule.getUint(message, &surfaceid);
    m_ipcModule.getUintArray(message, &array, &length);

    t_ilm_bool status = m_executor->execute(new SurfaceSetChromaKeyCommand(clientPid, surfaceid, array, length));
    if (status)
    {
        response = m_ipcModule.createResponse(message);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_NOT_FOUND);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::SetLayerChromaKey(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    uint* array = NULL;
    int length = 0;
    uint layerid = 0;

    m_ipcModule.getUint(message, &layerid);
    m_ipcModule.getUintArray(message, &array, &length);

    t_ilm_bool status = m_executor->execute(new LayerSetChromaKeyCommand(clientPid, layerid, array, length));
    if (status)
    {
        response = m_ipcModule.createResponse(message);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_NOT_FOUND);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::LayerAddNotification(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    uint layerid = 0;

    m_ipcModule.getUint(message, &layerid);

    Layer* layer = m_executor->getScene()->getLayer(layerid);
    if (layer)
    {
        layer->addNotification(clientHandle);
        response = m_ipcModule.createResponse(message);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, INVALID_ARGUMENT);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::SurfaceAddNotification(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    uint surfaceid = 0;

    m_ipcModule.getUint(message, &surfaceid);

    Surface* surface = m_executor->getScene()->getSurface(surfaceid);
    if (surface)
    {
        surface->addNotification(clientHandle);
        response = m_ipcModule.createResponse(message);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, INVALID_ARGUMENT);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::LayerRemoveNotification(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    uint layerid = 0;

    m_ipcModule.getUint(message, &layerid);

    Layer* layer = m_executor->getScene()->getLayer(layerid);
    if (layer)
    {
        layer->removeNotification(clientHandle);
        response = m_ipcModule.createResponse(message);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, INVALID_ARGUMENT);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::SurfaceRemoveNotification(t_ilm_message message)
{
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_message response;
    uint surfaceid = 0;

    m_ipcModule.getUint(message, &surfaceid);

    Surface* surface = m_executor->getScene()->getSurface(surfaceid);
    if (surface)
    {
        surface->removeNotification(clientHandle);
        response = m_ipcModule.createResponse(message);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, INVALID_ARGUMENT);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::processNotificationQueue()
{
    NotificationQueue& notificationQueue = m_executor->getClientNotificationQueue();
    NotificationQueue::iterator iter = notificationQueue.begin();
    NotificationQueue::iterator end = notificationQueue.end();

    for (; iter != end; ++iter)
    {
        GraphicalObject* object = iter->first;
        t_ilm_notification_mask mask = iter->second;
        sendNotification(object, mask);
    }
    notificationQueue.clear();
}

void GenericCommunicator::sendNotification(GraphicalObject* object, t_ilm_notification_mask mask)
{
    switch (object->type)
    {
    case TypeLayer:
    {
        Layer* layer = static_cast<Layer*>(object);
        if (layer)
        {
            ApplicationReferenceList& arl = layer->getNotificationClients();

            if (arl.size())
            {
                t_ilm_message notification;
                Rectangle dest = layer->getDestinationRegion();
                Rectangle src = layer->getSourceRegion();
                OrientationType orientation = layer->getOrientation();

                unsigned char chromaKeyRed = 0;
                unsigned char chromaKeyGreen = 0;
                unsigned char chromaKeyBlue = 0;
                layer->getChromaKey(chromaKeyRed, chromaKeyGreen, chromaKeyBlue);

                std::stringstream notificationName;
                notificationName << "NotificationForLayer" << layer->getID();

                notification = m_ipcModule.createNotification(notificationName.str().c_str());
                m_ipcModule.appendUint(notification, layer->getID());
                m_ipcModule.appendUint(notification, mask);

                m_ipcModule.appendDouble(notification, layer->getOpacity());
                m_ipcModule.appendUint(notification, src.x);
                m_ipcModule.appendUint(notification, src.y);
                m_ipcModule.appendUint(notification, src.width);
                m_ipcModule.appendUint(notification, src.height);
                m_ipcModule.appendUint(notification, layer->OriginalSourceWidth);
                m_ipcModule.appendUint(notification, layer->OriginalSourceHeight);
                m_ipcModule.appendUint(notification, dest.x);
                m_ipcModule.appendUint(notification, dest.y);
                m_ipcModule.appendUint(notification, dest.width);
                m_ipcModule.appendUint(notification, dest.height);
                m_ipcModule.appendUint(notification, orientation);
                m_ipcModule.appendBool(notification, layer->getVisibility());
                m_ipcModule.appendUint(notification, layer->getLayerType());
                m_ipcModule.appendBool(notification, layer->getChromaKeyEnabled());
                m_ipcModule.appendUint(notification, chromaKeyRed);
                m_ipcModule.appendUint(notification, chromaKeyGreen);
                m_ipcModule.appendUint(notification, chromaKeyBlue);
                m_ipcModule.appendInt(notification, layer->getCreatorPid());

                int clientCount = arl.size();
                t_ilm_client_handle clientArray[256];

                ApplicationReferenceList::iterator iter = arl.begin();
                ApplicationReferenceList::iterator end = arl.end();

                for (int clientNumber = 0;
                     iter != end, clientNumber < 256;
                     ++iter, ++clientNumber)
                {
                    t_ilm_client_handle client = *iter;
                    clientArray[clientNumber] = client;
                }

                LOG_DEBUG("GenericCommunicator", "Sending " << clientCount << " notification(s): layer " << layer->getID() << " was updated.");

                if (!m_ipcModule.sendToClients(notification, clientArray, clientCount))
                {
                    LOG_ERROR("GenericCommunicator", "Sending notification to clients failed.")
                }

                m_ipcModule.destroyMessage(notification);
            }
        }
    }
    break;
    case TypeSurface:
    {
        Surface* surface = static_cast<Surface*>(object);
        if (surface)
        {
            ApplicationReferenceList& arl = surface->getNotificationClients();

            if (arl.size())
            {
                t_ilm_message notification;
                std::stringstream notificationName;
                notificationName << "NotificationForSurface" << surface->getID();

                unsigned char chromaKeyRed = 0;
                unsigned char chromaKeyGreen = 0;
                unsigned char chromaKeyBlue = 0;
                surface->getChromaKey(chromaKeyRed, chromaKeyGreen, chromaKeyBlue);

                notification = m_ipcModule.createNotification(notificationName.str().c_str());
                m_ipcModule.appendUint(notification, surface->getID());
                m_ipcModule.appendUint(notification, mask);

                m_ipcModule.appendDouble(notification, surface->getOpacity());
                m_ipcModule.appendUint(notification, surface->getSourceRegion().x);
                m_ipcModule.appendUint(notification, surface->getSourceRegion().y);
                m_ipcModule.appendUint(notification, surface->getSourceRegion().width);
                m_ipcModule.appendUint(notification, surface->getSourceRegion().height);
                m_ipcModule.appendUint(notification, surface->OriginalSourceWidth);
                m_ipcModule.appendUint(notification, surface->OriginalSourceHeight);
                m_ipcModule.appendUint(notification, surface->getDestinationRegion().x);
                m_ipcModule.appendUint(notification, surface->getDestinationRegion().y);
                m_ipcModule.appendUint(notification, surface->getDestinationRegion().width);
                m_ipcModule.appendUint(notification, surface->getDestinationRegion().height);
                m_ipcModule.appendUint(notification, surface->getOrientation());
                m_ipcModule.appendBool(notification, surface->getVisibility());
                m_ipcModule.appendUint(notification, surface->frameCounter);
                m_ipcModule.appendUint(notification, surface->drawCounter);
                m_ipcModule.appendUint(notification, surface->updateCounter);
                m_ipcModule.appendUint(notification, surface->getPixelFormat());
                m_ipcModule.appendUint(notification, surface->getNativeContent());
                m_ipcModule.appendUint(notification, surface->getInputEventAcceptanceOnDevices());
                m_ipcModule.appendBool(notification, surface->getChromaKeyEnabled());
                m_ipcModule.appendUint(notification, chromaKeyRed);
                m_ipcModule.appendUint(notification, chromaKeyGreen);
                m_ipcModule.appendUint(notification, chromaKeyBlue);
                m_ipcModule.appendInt(notification, surface->getCreatorPid());

                int clientCount = arl.size();
                t_ilm_client_handle clients[256];

                ApplicationReferenceList::iterator iter = arl.begin();
                ApplicationReferenceList::iterator end = arl.end();

                for (int clientNumber = 0;
                     iter != end, clientNumber < 256;
                     ++iter, ++clientNumber)
                {
                    clients[clientNumber] = *iter;
                }

                LOG_DEBUG("GenericCommunicator", "Sending " << clientCount << " notification(s): surface " << surface->getID() << " was updated.");

                if (!m_ipcModule.sendToClients(notification, clients, clientCount))
                {
                    LOG_ERROR("GenericCommunicator", "Sending notification to clients failed.")
                }

                m_ipcModule.destroyMessage(notification);
            }
        }
    }
    break;
    default:
        LOG_INFO("GenericCommunicator", "Unknown notification found in queue.");
        break;
    }
}

HealthCondition GenericCommunicator::pluginGetHealth()
{
    HealthCondition health = PluginBase::pluginGetHealth();
    if (0 != pthread_kill(mThreadId, 0))
    {
        health = HealthDead;
    }
    return health;
}

void GenericCommunicator::SetOptimizationMode(t_ilm_message message)
{
    t_ilm_message response;
    OptimizationType optimizationId;
    OptimizationModeType optimizationMode;
    unsigned int o;    
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);

    m_ipcModule.getUint(message,&o);
    optimizationId = (OptimizationType) o;
    m_ipcModule.getUint(message,&o);
    optimizationMode = (OptimizationModeType) o;

    t_ilm_bool status = m_executor->execute(new SetOptimizationModeCommand(clientPid, optimizationId, optimizationMode));
    if (status)
    {
        response = m_ipcModule.createResponse(message);       
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response,RESOURCE_NOT_FOUND);
    }
    m_ipcModule.sendToClients(response,&clientHandle,1);
    m_ipcModule.destroyMessage(response);
}

void GenericCommunicator::GetOptimizationMode(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    t_ilm_uint clientPid = m_executor->getSenderPid(clientHandle);
    OptimizationType optimizationId;
    OptimizationModeType optimizationMode;
    unsigned int o;

    m_ipcModule.getUint(message,&o);
    optimizationId = (OptimizationType)o;

    t_ilm_bool status = m_executor->execute(new GetOptimizationModeCommand(clientPid, optimizationId, &optimizationMode));
    if (status)
    {
        
        response = m_ipcModule.createResponse(message);
        m_ipcModule.appendUint(response,(unsigned int)optimizationMode);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response,RESOURCE_NOT_FOUND);
    }
    m_ipcModule.sendToClients(response,&clientHandle,1);
    m_ipcModule.destroyMessage(response);

}

void GenericCommunicator::GetPropertiesOfScreen(t_ilm_message message)
{
    t_ilm_message response;
    t_ilm_client_handle clientHandle = m_ipcModule.getSenderHandle(message);
    uint id = 0;
    m_ipcModule.getUint(message, &id);

    LmScreen* screen = m_executor->getScene()->getScreen(id);
    if (screen != NULL)
    {
        LayerList renderOrder = screen->getCurrentRenderOrder();
        std::vector<t_ilm_layer> layerIdVector;

        LayerListConstIterator iter = renderOrder.begin();
        LayerListConstIterator end = renderOrder.end();
        for (; iter != end; ++iter)
        {
            layerIdVector.push_back((*iter)->getID());
        }

        uint numberOfHardwareLayers = m_executor->getNumberOfHardwareLayers(id);
        uint* resolution = m_executor->getScreenResolution(id);

        response = m_ipcModule.createResponse(message);
        m_ipcModule.appendUintArray(response, layerIdVector.data(), layerIdVector.size());
        m_ipcModule.appendUint(response,numberOfHardwareLayers);
        m_ipcModule.appendUint(response, resolution[0]);
        m_ipcModule.appendUint(response, resolution[1]);
    }
    else
    {
        response = m_ipcModule.createErrorResponse(message);
        m_ipcModule.appendString(response, RESOURCE_NOT_FOUND);
    }

    m_ipcModule.sendToClients(response, &clientHandle, 1);
    m_ipcModule.destroyMessage(response);
}

DECLARE_LAYERMANAGEMENT_PLUGIN(GenericCommunicator)

