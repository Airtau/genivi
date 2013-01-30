/**************************************************************************
 *
 * Copyright 2012 BMW Car IT GmbH
 * Copyright (C) 2012 DENSO CORPORATION and Robert Bosch Car Multimedia Gmbh
 * Copyright (C) 2012 Bayerische Motorenwerke Aktiengesellschaft
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
#include "ilm_client.h"
#include "ilm_types.h"
#include "IpcModuleLoader.h"
#include "ObjectType.h"
#include <pthread.h>
#include <sys/time.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>


//=============================================================================
// global settings
//=============================================================================
const int gReceiveTimeout  = -1;  // in ms, negative value for infinite
const int gResponseTimeout = 500; // in ms

// must be same as GraphicalObject::INVALID_ID, but this is defined in C++
// and can not be used here
#define INVALID_ID 0xFFFFFFFF

// queue names for incoming notifications and messages
#define NOTIFICATION_QUEUE_NAME "/ilmClient%dNotification"
#define INCOMING_QUEUE_NAME     "/ilmClient%dIncoming"

//=============================================================================
// global vars
//=============================================================================
extern char *__progname; // automatically gets assigned argv[0]

static struct IpcModule gIpcModule;

static pthread_t gReceiveThread;
static pthread_mutex_t gSendReceiveLock;
static pthread_t gNotificationThread;

static mqd_t incomingMqRead;
static mqd_t incomingMqWrite;

static mqd_t notificationMqRead;
static mqd_t notificationMqWrite;

static t_ilm_bool gInitialized = ILM_FALSE;

//=============================================================================
// notification management
//=============================================================================
#define MAX_CALLBACK_COUNT 64
struct
{
    t_ilm_uint id;
    layerNotificationFunc callback;
} static gLayerNotificationCallbacks[MAX_CALLBACK_COUNT];

struct
{
    t_ilm_uint id;
    surfaceNotificationFunc callback;
} static gSurfaceNotificationCallbacks[MAX_CALLBACK_COUNT];

void initNotificationCallbacks()
{
    int i = 0;
    for (i = 0; i < MAX_CALLBACK_COUNT; ++i)
    {
        gLayerNotificationCallbacks[i].id = INVALID_ID;
        gLayerNotificationCallbacks[i].callback = NULL;
        gSurfaceNotificationCallbacks[i].id = INVALID_ID;
        gSurfaceNotificationCallbacks[i].callback = NULL;
    }
}

layerNotificationFunc getLayerNotificationCallback(t_ilm_layer layer)
{
    int i = 0;
    for (i = 0; i < MAX_CALLBACK_COUNT; ++i)
    {
        if (gLayerNotificationCallbacks[i].id == layer)
        {
            return gLayerNotificationCallbacks[i].callback;
        }
    }
    return NULL;
}

surfaceNotificationFunc getSurfaceNotificationCallback(t_ilm_surface surface)
{
    int i = 0;
    for (i = 0; i < MAX_CALLBACK_COUNT; ++i)
    {
        if (gSurfaceNotificationCallbacks[i].id == surface)
        {
            return gSurfaceNotificationCallbacks[i].callback;
        }
    }
    return NULL;
}

t_ilm_bool findLayerCallback(t_ilm_layer layer)
{
    int i = 0;

    // try to overwrite existing entry for layer id
    for (i = 0; i < MAX_CALLBACK_COUNT; ++i)
    {
        if (gLayerNotificationCallbacks[i].id == layer)
        {
            return ILM_TRUE;
        }
    }
    return ILM_FALSE;
}

t_ilm_bool addLayerCallback(t_ilm_layer layer, layerNotificationFunc func)
{
    int i = 0;

    if (findLayerCallback(layer))
    {
        return ILM_FALSE;
    }

    // find free slot and store callback
    for (i = 0; i < MAX_CALLBACK_COUNT; ++i)
    {
        if (gLayerNotificationCallbacks[i].id == INVALID_ID)
        {
            gLayerNotificationCallbacks[i].id = layer;
            gLayerNotificationCallbacks[i].callback = func;
            return ILM_TRUE;
        }
    }
    printf("DbusIpcModule: addLayerCallback() failed. no free slots.");
    return ILM_FALSE;
}

t_ilm_bool findSurfaceCallback(t_ilm_surface surface)
{
    int i = 0;

    // try to overwrite existing entry for layer id
    for (i = 0; i < MAX_CALLBACK_COUNT; ++i)
    {
        if (gSurfaceNotificationCallbacks[i].id == surface)
        {
            return ILM_TRUE;
        }
    }
    return ILM_FALSE;
}

t_ilm_bool addSurfaceCallback(t_ilm_surface surface, surfaceNotificationFunc func)
{
    int i = 0;

    if (findSurfaceCallback(surface))
    {
        return ILM_FALSE;
    }

    // find free slot and store callback
    for (i = 0; i < MAX_CALLBACK_COUNT; ++i)
    {
        if (gSurfaceNotificationCallbacks[i].id == INVALID_ID)
        {
            gSurfaceNotificationCallbacks[i].id = surface;
            gSurfaceNotificationCallbacks[i].callback = func;
            return ILM_TRUE;
        }
    }
    printf("DbusIpcModule: addSurfaceCallback() failed. no free slots.");
    return ILM_FALSE;
}

void removeLayerCallback(t_ilm_layer layer)
{
    int i = 0;
    for (i = 0; i < MAX_CALLBACK_COUNT; ++i)
    {
        if (gLayerNotificationCallbacks[i].id == layer)
        {
            gLayerNotificationCallbacks[i].id = INVALID_ID;
            gLayerNotificationCallbacks[i].callback = NULL;
            return;
        }
    }
}

void removeSurfaceCallback(t_ilm_surface layer)
{
    int i = 0;
    for (i = 0; i < MAX_CALLBACK_COUNT; ++i)
    {
        if (gSurfaceNotificationCallbacks[i].id == layer)
        {
            gSurfaceNotificationCallbacks[i].id = INVALID_ID;
            gSurfaceNotificationCallbacks[i].callback = NULL;
            return;
        }
    }
}

//=============================================================================
// handling of internal notification thread for dispatching notifications
// Note: notification callbacks may be blocked by client, but receive thread
// must not be blocked
//=============================================================================
void* notificationThreadLoop(void* param)
{
    (void)param;

    t_ilm_message notification;

    while (-1 != mq_receive(notificationMqRead, (char*)&notification, sizeof(notification), NULL))
    {
        t_ilm_const_string name = gIpcModule.getMessageName(notification);

        // this depends on message name, but it is fast
        if ('L' == name[15])
        {
            t_ilm_uint id;
            t_ilm_uint mask;
            struct ilmLayerProperties properties;

            gIpcModule.getUint(notification, &id);
            gIpcModule.getUint(notification, &mask);
            gIpcModule.getDouble(notification, &properties.opacity);
            gIpcModule.getUint(notification, &properties.sourceX);
            gIpcModule.getUint(notification, &properties.sourceY);
            gIpcModule.getUint(notification, &properties.sourceWidth);
            gIpcModule.getUint(notification, &properties.sourceHeight);
            gIpcModule.getUint(notification, &properties.origSourceWidth);
            gIpcModule.getUint(notification, &properties.origSourceHeight);
            gIpcModule.getUint(notification, &properties.destX);
            gIpcModule.getUint(notification, &properties.destY);
            gIpcModule.getUint(notification, &properties.destWidth);
            gIpcModule.getUint(notification, &properties.destHeight);
            gIpcModule.getUint(notification, &properties.orientation);
            gIpcModule.getBool(notification, &properties.visibility);
            gIpcModule.getUint(notification, &properties.type);
            gIpcModule.getBool(notification, &properties.chromaKeyEnabled);
            gIpcModule.getUint(notification, &properties.chromaKeyRed);
            gIpcModule.getUint(notification, &properties.chromaKeyGreen);
            gIpcModule.getUint(notification, &properties.chromaKeyBlue);
            gIpcModule.getInt(notification, &properties.creatorPid);

            layerNotificationFunc func = getLayerNotificationCallback(id);
            if (func)
            {
                (*func)(id, &properties, mask);
            }
            else
            {
                fprintf(stderr, "notification for layer %d received, but no callback set\n", id);
            }
        }

        if ('S' == name[15])
        {
            t_ilm_uint id;
            t_ilm_uint mask;
            struct ilmSurfaceProperties properties;

            gIpcModule.getUint(notification, &id);
            gIpcModule.getUint(notification, &mask);
            gIpcModule.getDouble(notification, &properties.opacity);
            gIpcModule.getUint(notification, &properties.sourceX);
            gIpcModule.getUint(notification, &properties.sourceY);
            gIpcModule.getUint(notification, &properties.sourceWidth);
            gIpcModule.getUint(notification, &properties.sourceHeight);
            gIpcModule.getUint(notification, &properties.origSourceWidth);
            gIpcModule.getUint(notification, &properties.origSourceHeight);
            gIpcModule.getUint(notification, &properties.destX);
            gIpcModule.getUint(notification, &properties.destY);
            gIpcModule.getUint(notification, &properties.destWidth);
            gIpcModule.getUint(notification, &properties.destHeight);
            gIpcModule.getUint(notification, &properties.orientation);
            gIpcModule.getBool(notification, &properties.visibility);
            gIpcModule.getUint(notification, &properties.frameCounter);
            gIpcModule.getUint(notification, &properties.drawCounter);
            gIpcModule.getUint(notification, &properties.updateCounter);
            gIpcModule.getUint(notification, &properties.pixelformat);
            gIpcModule.getUint(notification, &properties.nativeSurface);
            gIpcModule.getUint(notification, &properties.inputDevicesAcceptance);
            gIpcModule.getBool(notification, &properties.chromaKeyEnabled);
            gIpcModule.getUint(notification, &properties.chromaKeyRed);
            gIpcModule.getUint(notification, &properties.chromaKeyGreen);
            gIpcModule.getUint(notification, &properties.chromaKeyBlue);
            gIpcModule.getInt(notification, &properties.creatorPid);

            surfaceNotificationFunc func = getSurfaceNotificationCallback(id);
            if (func)
            {
                (*func)(id, &properties, mask);
            }
            else
            {
                fprintf(stderr, "notification for surface %d received, but no callback set\n", id);
            }
        }
        gIpcModule.destroyMessage(notification);
    }
    return NULL;
}

//=============================================================================
// handling of internal receive thread for event handling
//=============================================================================
void* receiveThreadLoop(void* param)
{
    (void)param;

    t_ilm_bool running = ILM_TRUE;

    while (running)
    {
        t_ilm_message message = gIpcModule.receive(gReceiveTimeout);
        t_ilm_message_type messageType = gIpcModule.getMessageType(message);
        switch (messageType)
        {
            case IpcMessageTypeNotification:
                if (-1 == mq_send(notificationMqWrite, (char*)&message, sizeof(message), 0))
                {
                    if (EAGAIN == errno)
                    {
                        printf("Notification queue full, dropped notification %s\n", gIpcModule.getMessageName(message));
                    }
                }
                break;

            case IpcMessageTypeCommand:
            case IpcMessageTypeError:
                if (-1 == mq_send(incomingMqWrite, (char*)&message, sizeof(message), 0))
                {
                    if (EAGAIN == errno)
                    {
                        printf("Incoming queue full, dropped message %s\n", gIpcModule.getMessageName(message));
                    }
                }
                break;

            case IpcMessageTypeNone:
                break;

            default:
                printf("ilmClient: discarded unexpected message (type: %d)\n", (int)messageType);
                gIpcModule.destroyMessage(message);
                break;
        }
    }

    return NULL;
}

void calculateTimeout(struct timeval* currentTime, int giventimeout, struct timespec* timeout)
{
    // nanoseconds is old value in nanoseconds + the given milliseconds as nanoseconds
    t_ilm_ulong newNanoSeconds = currentTime->tv_usec * 1000 + giventimeout * (1000 * 1000);

    // only use non full seconds, otherwise overflow!
    timeout->tv_nsec = newNanoSeconds % (1000000000);

    // new seconds are old seconds + full seconds from new nanoseconds part
    timeout->tv_sec  = currentTime->tv_sec + (newNanoSeconds / 1000000000 );
}

t_ilm_bool sendAndWaitForResponse(t_ilm_message command, t_ilm_message* response, int timeoutInMs)
{
    (void)timeoutInMs;

    *response = 0;
    t_ilm_message_type responseType = IpcMessageTypeNone;

    // send / receive may only be performed by one thread at a time
    pthread_mutex_lock(&gSendReceiveLock);

    if (gIpcModule.sendToService(command))
    {
        if (-1 == mq_receive(incomingMqRead, (char*)response, sizeof(t_ilm_message), NULL))
        {
            fprintf(stderr,"waitForResponse: mq_receive failed, errno = %d\n", errno);
        }
        else
        {
            responseType = gIpcModule.getMessageType(*response);
        }
    }
    pthread_mutex_unlock(&gSendReceiveLock);

    return (*response && (IpcMessageTypeCommand == responseType));
}

//=============================================================================
// implementation
//=============================================================================
ilmErrorTypes ilm_init()
{
    ilmErrorTypes result = ILM_FAILED;

    if (gInitialized)
    {
        printf("ilm_init() was called, but ilmClientLib is already initialized. returning success, but initialization was skipped this time.\n");
        return ILM_SUCCESS;
    }

    initNotificationCallbacks();

    if (loadIpcModule(&gIpcModule))
    {
        int pid = getpid();

        if (gIpcModule.initClientMode())
        {
            result = ILM_SUCCESS;

            struct mq_attr mqAttr =
            {
                .mq_maxmsg = 4,
                .mq_msgsize = sizeof(t_ilm_message),
                .mq_flags = 0, //O_NONBLOCK,
                .mq_curmsgs = 0
            };

            char mqName[30];
            snprintf(mqName, sizeof(mqName), NOTIFICATION_QUEUE_NAME, getpid());

            notificationMqWrite = mq_open(mqName, O_WRONLY | O_CREAT, 0600, &mqAttr);
            notificationMqRead = mq_open(mqName, O_RDONLY);
            mq_unlink(mqName); // is destroyed on closed filedescriptor

            snprintf(mqName, sizeof(mqName), INCOMING_QUEUE_NAME, getpid());
            incomingMqWrite = mq_open(mqName, O_WRONLY | O_CREAT, 0600, &mqAttr);
            incomingMqRead = mq_open(mqName, O_RDONLY);
            mq_unlink(mqName); // is destroyed on closed filedescriptor

            if ((mqd_t)-1 == notificationMqRead || (mqd_t)-1 == notificationMqWrite)
            {
                printf("mq_open failed, errno = %d\n", errno);
                return result;
            }

            pthread_mutex_init(&gSendReceiveLock, NULL);

            pthread_attr_t notificationThreadAttributes;
            pthread_attr_init(&notificationThreadAttributes);
            pthread_attr_setdetachstate(&notificationThreadAttributes,
                                        PTHREAD_CREATE_JOINABLE);

            int ret = pthread_create(&gReceiveThread,
                                     &notificationThreadAttributes,
                                     receiveThreadLoop,
                                     NULL);
            if (0 != ret)
            {
                result = ILM_FAILED;
                printf("Failed to start internal receive thread. returned %d = %s\n", ret, (ret == EAGAIN ? "EAGAIN" : "EINVAL"));
                return result;
            }

            ret = pthread_create(&gNotificationThread,
                                 &notificationThreadAttributes,
                                 notificationThreadLoop,
                                 NULL);
            if (0 != ret)
            {
                result = ILM_FAILED;
                printf("Failed to start internal notification thread. returned %d = %s\n", ret, (ret == EAGAIN ? "EAGAIN" : "EINVAL"));
                return result;
            }
        }
        else
        {
            result = ILM_FAILED;
            printf("Failed to initialize Client Ipc Module");
            return result;
        }

        t_ilm_message response = 0;
        t_ilm_message command = gIpcModule.createMessage("ServiceConnect");
        if (command
                && gIpcModule.appendUint(command, pid)
                && gIpcModule.appendString(command, __progname)
                && sendAndWaitForResponse(command, &response, gResponseTimeout))
        {
            result = ILM_SUCCESS;
        }
        else
        {
            result = ILM_FAILED;
            printf("Failed to connect to LayerManagerService.");
        }
        gIpcModule.destroyMessage(response);
        gIpcModule.destroyMessage(command);
    }

    gInitialized = (result == ILM_SUCCESS) ? ILM_TRUE : ILM_FALSE;

    return result;
}

ilmErrorTypes ilm_destroy()
{
    ilmErrorTypes result = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("ServiceDisconnect");
    if (command
        && gIpcModule.appendUint(command, getpid())
        && sendAndWaitForResponse(command, &response, gResponseTimeout))
    {
        result = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);

    // cancel worker threads
    void* threadReturnValue = NULL;

    pthread_cancel(gReceiveThread);
    pthread_cancel(gNotificationThread);

    pthread_join(gReceiveThread, &threadReturnValue);
    pthread_join(gNotificationThread, &threadReturnValue);

    pthread_mutex_unlock(&gSendReceiveLock);

    pthread_mutex_destroy(&gSendReceiveLock);

    gIpcModule.destroy();

    mq_close(notificationMqRead);
    mq_close(notificationMqWrite);

    mq_close(incomingMqRead);
    mq_close(incomingMqWrite);

    gInitialized = ILM_FALSE;

    return result;
}

ilmErrorTypes ilm_getPropertiesOfSurface(t_ilm_uint surfaceID, struct ilmSurfaceProperties* pSurfaceProperties)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("GetPropertiesOfSurface");

    if (pSurfaceProperties
        && command
        && gIpcModule.appendUint(command, surfaceID)
        && sendAndWaitForResponse(command, &response, gResponseTimeout)
        && gIpcModule.getDouble(response, &pSurfaceProperties->opacity)
        && gIpcModule.getUint(response, &pSurfaceProperties->sourceX)
        && gIpcModule.getUint(response, &pSurfaceProperties->sourceY)
        && gIpcModule.getUint(response, &pSurfaceProperties->sourceWidth)
        && gIpcModule.getUint(response, &pSurfaceProperties->sourceHeight)
        && gIpcModule.getUint(response, &pSurfaceProperties->origSourceWidth)
        && gIpcModule.getUint(response, &pSurfaceProperties->origSourceHeight)
        && gIpcModule.getUint(response, &pSurfaceProperties->destX)
        && gIpcModule.getUint(response, &pSurfaceProperties->destY)
        && gIpcModule.getUint(response, &pSurfaceProperties->destWidth)
        && gIpcModule.getUint(response, &pSurfaceProperties->destHeight)
        && gIpcModule.getUint(response, &pSurfaceProperties->orientation)
        && gIpcModule.getBool(response, &pSurfaceProperties->visibility)
        && gIpcModule.getUint(response, &pSurfaceProperties->frameCounter)
        && gIpcModule.getUint(response, &pSurfaceProperties->drawCounter)
        && gIpcModule.getUint(response, &pSurfaceProperties->updateCounter)
        && gIpcModule.getUint(response, &pSurfaceProperties->pixelformat)
        && gIpcModule.getUint(response, &pSurfaceProperties->nativeSurface)
        && gIpcModule.getUint(response, &pSurfaceProperties->inputDevicesAcceptance)
        && gIpcModule.getBool(response, &pSurfaceProperties->chromaKeyEnabled)
        && gIpcModule.getUint(response, &pSurfaceProperties->chromaKeyRed)
        && gIpcModule.getUint(response, &pSurfaceProperties->chromaKeyGreen)
        && gIpcModule.getUint(response, &pSurfaceProperties->chromaKeyBlue)
        && gIpcModule.getInt(response, &pSurfaceProperties->creatorPid))
    {
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_getPropertiesOfLayer(t_ilm_uint layerID, struct ilmLayerProperties* pLayerProperties)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("GetPropertiesOfLayer");
    if (pLayerProperties
        && command
        && gIpcModule.appendUint(command, layerID)
        && sendAndWaitForResponse(command, &response, gResponseTimeout)
        && gIpcModule.getDouble(response, &pLayerProperties->opacity)
        && gIpcModule.getUint(response, &pLayerProperties->sourceX)
        && gIpcModule.getUint(response, &pLayerProperties->sourceY)
        && gIpcModule.getUint(response, &pLayerProperties->sourceWidth)
        && gIpcModule.getUint(response, &pLayerProperties->sourceHeight)
        && gIpcModule.getUint(response, &pLayerProperties->origSourceWidth)
        && gIpcModule.getUint(response, &pLayerProperties->origSourceHeight)
        && gIpcModule.getUint(response, &pLayerProperties->destX)
        && gIpcModule.getUint(response, &pLayerProperties->destY)
        && gIpcModule.getUint(response, &pLayerProperties->destWidth)
        && gIpcModule.getUint(response, &pLayerProperties->destHeight)
        && gIpcModule.getUint(response, &pLayerProperties->orientation)
        && gIpcModule.getBool(response, &pLayerProperties->visibility)
        && gIpcModule.getUint(response, &pLayerProperties->type)
        && gIpcModule.getBool(response, &pLayerProperties->chromaKeyEnabled)
        && gIpcModule.getUint(response, &pLayerProperties->chromaKeyRed)
        && gIpcModule.getUint(response, &pLayerProperties->chromaKeyGreen)
        && gIpcModule.getUint(response, &pLayerProperties->chromaKeyBlue)
        && gIpcModule.getInt(response, &pLayerProperties->creatorPid))
    {
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_getNumberOfHardwareLayers(t_ilm_uint screenID, t_ilm_uint* pNumberOfHardwareLayers)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("GetNumberOfHardwareLayers");
    if (pNumberOfHardwareLayers
        && command
        && gIpcModule.appendUint(command, screenID)
        && sendAndWaitForResponse(command, &response, gResponseTimeout)
        && gIpcModule.getUint(response, pNumberOfHardwareLayers))
    {
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_getScreenResolution(t_ilm_uint screenID, t_ilm_uint* pWidth, t_ilm_uint* pHeight)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("GetScreenResolution");
    if (pWidth && pHeight
        && command
        && gIpcModule.appendUint(command, screenID)
        && sendAndWaitForResponse(command, &response, gResponseTimeout)
        && gIpcModule.getUint(response, pWidth)
        && gIpcModule.getUint(response, pHeight))
    {
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_getLayerIDs(t_ilm_int* pLength, t_ilm_layer** ppArray)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("ListAllLayerIDS");
    if (pLength && ppArray
        && command
        && sendAndWaitForResponse(command, &response, gResponseTimeout)
        && gIpcModule.getUintArray(response, ppArray, pLength))
    {
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_getLayerIDsOnScreen(t_ilm_uint screenId, t_ilm_int* pLength, t_ilm_layer** ppArray)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("ListAllLayerIDsOnScreen");
    if (pLength && ppArray
        && command
        && gIpcModule.appendUint(command, screenId)
        && sendAndWaitForResponse(command, &response, gResponseTimeout)
        && gIpcModule.getUintArray(response, ppArray, pLength))
    {
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_getSurfaceIDs(t_ilm_int* pLength, t_ilm_surface** ppArray)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("ListAllSurfaceIDS");
    if (pLength && ppArray
        && command
        && sendAndWaitForResponse(command, &response, gResponseTimeout)
        && gIpcModule.getUintArray(response, ppArray, pLength))
    {
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_getSurfaceIDsOnLayer(t_ilm_layer layer, t_ilm_int* pLength, t_ilm_surface** ppArray)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("ListSurfaceofLayer");
    if (pLength && ppArray
        && command
        && gIpcModule.appendUint(command, layer)
        && sendAndWaitForResponse(command, &response, gResponseTimeout)
        && gIpcModule.getUintArray(response, ppArray, pLength))
    {
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_layerCreate(t_ilm_layer* pLayerId)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    if (pLayerId && (INVALID_ID != *pLayerId))
    {
        t_ilm_message response = 0;
        t_ilm_message command = gIpcModule.createMessage("CreateLayerFromId");
        if (command
            && gIpcModule.appendUint(command, *pLayerId)
            && sendAndWaitForResponse(command, &response, gResponseTimeout)
            && gIpcModule.getUint(response, pLayerId))
        {
            returnValue = ILM_SUCCESS;
        }
        gIpcModule.destroyMessage(response);
        gIpcModule.destroyMessage(command);
    }
    else
    {
        t_ilm_message response = 0;
        t_ilm_message command = gIpcModule.createMessage("CreateLayer");
        if (command
            && sendAndWaitForResponse(command, &response, gResponseTimeout)
            && gIpcModule.getUint(response, pLayerId))
        {
            returnValue = ILM_SUCCESS;
        }
        gIpcModule.destroyMessage(response);
        gIpcModule.destroyMessage(command);
    }
    return returnValue;
}

ilmErrorTypes ilm_layerCreateWithDimension(t_ilm_layer* pLayerId, t_ilm_uint width, t_ilm_uint height)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    if (pLayerId && (INVALID_ID != *pLayerId))
    {
        t_ilm_message response = 0;
        t_ilm_message command = gIpcModule.createMessage("CreateLayerFromIdWithDimension");
        if (command
            && gIpcModule.appendUint(command, *pLayerId)
            && gIpcModule.appendUint(command, width)
            && gIpcModule.appendUint(command, height)
            && sendAndWaitForResponse(command, &response, gResponseTimeout)
            && gIpcModule.getUint(response, pLayerId))
        {
            returnValue = ILM_SUCCESS;
        }
        gIpcModule.destroyMessage(response);
        gIpcModule.destroyMessage(command);
    }
    else
    {
        t_ilm_message response = 0;
        t_ilm_message command = gIpcModule.createMessage("CreateLayerWithDimension");
        if (command
            && gIpcModule.appendUint(command, width)
            && gIpcModule.appendUint(command, height)
            && sendAndWaitForResponse(command, &response, gResponseTimeout)
            && gIpcModule.getUint(response, pLayerId))
        {
            returnValue = ILM_SUCCESS;
        }
        gIpcModule.destroyMessage(response);
        gIpcModule.destroyMessage(command);
    }
    return returnValue;
}

ilmErrorTypes ilm_layerRemove(t_ilm_layer layerId)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("RemoveLayer");
    if (command
        && gIpcModule.appendUint(command, layerId)
        && sendAndWaitForResponse(command, &response, gResponseTimeout))
    {
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_layerAddSurface(t_ilm_layer layerId, t_ilm_surface surfaceId)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("AddSurfaceToLayer");
    if (command
        && gIpcModule.appendUint(command, surfaceId)
        && gIpcModule.appendUint(command, layerId)
        && sendAndWaitForResponse(command, &response, gResponseTimeout))
    {
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_layerRemoveSurface(t_ilm_layer layerId, t_ilm_surface surfaceId)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("RemoveSurfaceFromLayer");
    if (command
        && gIpcModule.appendUint(command, surfaceId)
        && gIpcModule.appendUint(command, layerId)
        && sendAndWaitForResponse(command, &response, gResponseTimeout))
    {
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_layerGetType(t_ilm_layer layerId, ilmLayerType* pLayerType)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("GetLayerType");
    if (pLayerType
        && command
        && gIpcModule.appendUint(command, layerId)
        && sendAndWaitForResponse(command, &response, gResponseTimeout)
        && gIpcModule.getUint(response, pLayerType))
    {
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_layerSetVisibility(t_ilm_layer layerId, t_ilm_bool newVisibility)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("SetLayerVisibility");
    if (command
        && gIpcModule.appendUint(command, layerId)
        && gIpcModule.appendBool(command, newVisibility)
        && sendAndWaitForResponse(command, &response, gResponseTimeout))
    {
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_layerGetVisibility(t_ilm_layer layerId, t_ilm_bool *pVisibility)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("GetLayerVisibility");
    if (pVisibility
        && command
        && gIpcModule.appendUint(command, layerId)
        && sendAndWaitForResponse(command, &response, gResponseTimeout)
        && gIpcModule.getBool(response, pVisibility))
    {
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_layerSetOpacity(t_ilm_layer layerId, t_ilm_float opacity)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("SetLayerOpacity");
    if (command
        && gIpcModule.appendUint(command, layerId)
        && gIpcModule.appendDouble(command, opacity)
        && sendAndWaitForResponse(command, &response, gResponseTimeout))
    {
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_layerGetOpacity(t_ilm_layer layerId, t_ilm_float *pOpacity)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("GetLayerOpacity");
    if (pOpacity
        && command
        && gIpcModule.appendUint(command, layerId)
        && sendAndWaitForResponse(command, &response, gResponseTimeout)
        && gIpcModule.getDouble(response, pOpacity))
    {
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_layerSetSourceRectangle(t_ilm_layer layerId, t_ilm_uint x, t_ilm_uint y, t_ilm_uint width, t_ilm_uint height)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("SetLayerSourceRegion");
    if (command
        && gIpcModule.appendUint(command, layerId)
        && gIpcModule.appendUint(command, x)
        && gIpcModule.appendUint(command, y)
        && gIpcModule.appendUint(command, width)
        && gIpcModule.appendUint(command, height)
        && sendAndWaitForResponse(command, &response, gResponseTimeout))
    {
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_layerSetDestinationRectangle(t_ilm_layer layerId, t_ilm_int x, t_ilm_int y, t_ilm_int width, t_ilm_int height)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("SetLayerDestinationRegion");
    if (command
        && gIpcModule.appendUint(command, layerId)
        && gIpcModule.appendUint(command, x)
        && gIpcModule.appendUint(command, y)
        && gIpcModule.appendUint(command, width)
        && gIpcModule.appendUint(command, height)
        && sendAndWaitForResponse(command, &response, gResponseTimeout))
    {
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_layerGetDimension(t_ilm_layer layerId, t_ilm_uint *pDimension)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("GetLayerDimension");
    if (pDimension
        && command
        && gIpcModule.appendUint(command, layerId)
        && sendAndWaitForResponse(command, &response, gResponseTimeout)
        && gIpcModule.getUint(response, &pDimension[0])
        && gIpcModule.getUint(response, &pDimension[1]))
    {
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_layerSetDimension(t_ilm_layer layerId, t_ilm_uint *pDimension)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("SetLayerDimension");
    if (pDimension
        && command
        && gIpcModule.appendUint(command, layerId)
        && gIpcModule.appendUint(command, pDimension[0])
        && gIpcModule.appendUint(command, pDimension[1])
        && sendAndWaitForResponse(command, &response, gResponseTimeout))
    {
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_layerGetPosition(t_ilm_layer layerId, t_ilm_uint *pPosition)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("GetLayerPosition");
    if (pPosition
        && command
        && gIpcModule.appendUint(command, layerId)
        && sendAndWaitForResponse(command, &response, gResponseTimeout)
        && gIpcModule.getUint(response, &pPosition[0])
        && gIpcModule.getUint(response, &pPosition[1]))
    {
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_layerSetPosition(t_ilm_layer layerId, t_ilm_uint *pPosition)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("SetLayerPosition");
    if (pPosition
        && command
        && gIpcModule.appendUint(command, layerId)
        && gIpcModule.appendUint(command, pPosition[0])
        && gIpcModule.appendUint(command, pPosition[1])
        && sendAndWaitForResponse(command, &response, gResponseTimeout))
    {
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_layerSetOrientation(t_ilm_layer layerId, ilmOrientation orientation)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("SetLayerOrientation");
    if (command
        && gIpcModule.appendUint(command, layerId)
        && gIpcModule.appendUint(command, orientation)
        && sendAndWaitForResponse(command, &response, gResponseTimeout))
    {
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_layerGetOrientation(t_ilm_layer layerId, ilmOrientation *pOrientation)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("GetLayerOrientation");
    if (pOrientation
        && command
        && gIpcModule.appendUint(command, layerId)
        && sendAndWaitForResponse(command, &response, gResponseTimeout)
        && gIpcModule.getUint(response, pOrientation))
    {
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_layerSetChromaKey(t_ilm_layer layerId, t_ilm_int* pColor)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("SetLayerChromaKey");
    if (command
        && gIpcModule.appendUint(command, layerId))
    {
        t_ilm_bool comResult = ILM_TRUE;

        // Checking pColor has a content, otherwise chromakey is disabled
        if (pColor)
        {
            const t_ilm_uint number = 3;
            comResult = gIpcModule.appendUintArray(command, (t_ilm_uint *)pColor, number);
        }
        if (comResult
            && sendAndWaitForResponse(command, &response, gResponseTimeout))
        {
            returnValue = ILM_SUCCESS;
        }
        gIpcModule.destroyMessage(response);
    }
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_layerSetRenderOrder(t_ilm_layer layerId, t_ilm_layer *pSurfaceId, t_ilm_int number)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("SetSurfaceRenderOrderWithinLayer");
    if (pSurfaceId
        && command
        && gIpcModule.appendUint(command, layerId)
        && gIpcModule.appendUintArray(command, pSurfaceId, number)
        && sendAndWaitForResponse(command, &response, gResponseTimeout))
    {
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_layerGetCapabilities(t_ilm_layer layerId, t_ilm_layercapabilities *pCapabilities)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("GetLayerCapabilities");
    if (pCapabilities
        && command
        && gIpcModule.appendUint(command, layerId)
        && sendAndWaitForResponse(command, &response, gResponseTimeout)
        && gIpcModule.getUint(response, pCapabilities))
    {
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_layerTypeGetCapabilities(ilmLayerType layerType, t_ilm_layercapabilities *pCapabilities)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("GetLayertypeCapabilities");
    if (pCapabilities
        && command
        && gIpcModule.appendUint(command, layerType)
        && sendAndWaitForResponse(command, &response, gResponseTimeout)
        && gIpcModule.getUint(response, pCapabilities))
    {
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_surfaceCreate(t_ilm_nativehandle nativehandle, t_ilm_int width, t_ilm_int height, ilmPixelFormat pixelFormat, t_ilm_surface* pSurfaceId)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    if (pSurfaceId && (INVALID_ID != *pSurfaceId))
    {
        t_ilm_message response = 0;
        t_ilm_message command = gIpcModule.createMessage("CreateSurfaceFromId");
        if (command
            && gIpcModule.appendUint(command, nativehandle)
            && gIpcModule.appendUint(command, width)
            && gIpcModule.appendUint(command, height)
            && gIpcModule.appendUint(command, pixelFormat)
            && gIpcModule.appendUint(command, *pSurfaceId)
            && sendAndWaitForResponse(command, &response, gResponseTimeout)
            && gIpcModule.getUint(response, pSurfaceId))
        {
            returnValue = ILM_SUCCESS;
        }
        gIpcModule.destroyMessage(response);
        gIpcModule.destroyMessage(command);
    }
    else
    {
        t_ilm_message response = 0;
        t_ilm_message command = gIpcModule.createMessage("CreateSurface");
        if (command
            && gIpcModule.appendUint(command, nativehandle)
            && gIpcModule.appendUint(command, width)
            && gIpcModule.appendUint(command, height)
            && gIpcModule.appendUint(command, pixelFormat)
            && sendAndWaitForResponse(command, &response, gResponseTimeout)
            && gIpcModule.getUint(response, pSurfaceId))
        {
            returnValue = ILM_SUCCESS;
        }
        gIpcModule.destroyMessage(response);
        gIpcModule.destroyMessage(command);
    }
    return returnValue;
}

ilmErrorTypes ilm_surfaceInitialize(t_ilm_surface *pSurfaceId)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    if (pSurfaceId && (INVALID_ID != *pSurfaceId))
    {
        t_ilm_message response = 0;
        t_ilm_message command = gIpcModule.createMessage("InitializeSurfaceFromId");
        if (command
            && gIpcModule.appendUint(command, *pSurfaceId)
            && sendAndWaitForResponse(command, &response, gResponseTimeout)
            && gIpcModule.getUint(response, pSurfaceId))
        {
            returnValue = ILM_SUCCESS;
        }
        gIpcModule.destroyMessage(response);
        gIpcModule.destroyMessage(command);
    }
    else
    {
        t_ilm_message response = 0;
        t_ilm_message command = gIpcModule.createMessage("InitializeSurface");
        if (command
            && sendAndWaitForResponse(command, &response, gResponseTimeout)
            && gIpcModule.getUint(response, pSurfaceId))
        {
            returnValue = ILM_SUCCESS;
        }
        gIpcModule.destroyMessage(response);
        gIpcModule.destroyMessage(command);
    }
    return returnValue;
}

ilmErrorTypes ilm_surfaceSetNativeContent(t_ilm_nativehandle nativehandle, t_ilm_int width, t_ilm_int height, ilmPixelFormat pixelFormat, t_ilm_surface surfaceId)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("SetSurfaceNativeContent");
    if (command
        && gIpcModule.appendUint(command, surfaceId)
        && gIpcModule.appendUint(command, nativehandle)
        && gIpcModule.appendUint(command, width)
        && gIpcModule.appendUint(command, height)
        && gIpcModule.appendUint(command, pixelFormat)
        && sendAndWaitForResponse(command, &response, gResponseTimeout))
    {
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_surfaceRemoveNativeContent(t_ilm_surface surfaceId)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("RemoveSurfaceNativeContent");
    if (command
        && gIpcModule.appendUint(command, surfaceId)
        && sendAndWaitForResponse(command, &response, gResponseTimeout))
    {
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_surfaceRemove(t_ilm_surface surfaceId)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("RemoveSurface");
    if (command
        && gIpcModule.appendUint(command, surfaceId)
        && sendAndWaitForResponse(command, &response, gResponseTimeout))
    {
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_surfaceSetVisibility(t_ilm_surface surfaceId, t_ilm_bool newVisibility)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("SetSurfaceVisibility");
    if (command
        && gIpcModule.appendUint(command, surfaceId)
        && gIpcModule.appendBool(command, newVisibility)
        && sendAndWaitForResponse(command, &response, gResponseTimeout))
    {
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_surfaceGetVisibility(t_ilm_surface surfaceId, t_ilm_bool *pVisibility)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("GetSurfaceVisibility");
    if (pVisibility
        && command
        && gIpcModule.appendUint(command, surfaceId)
        && sendAndWaitForResponse(command, &response, gResponseTimeout)
        && gIpcModule.getBool(response, pVisibility))
    {
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_surfaceSetOpacity(t_ilm_surface surfaceId, t_ilm_float opacity)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("SetSurfaceOpacity");
    if (command
        && gIpcModule.appendUint(command, surfaceId)
        && gIpcModule.appendDouble(command, opacity)
        && sendAndWaitForResponse(command, &response, gResponseTimeout))
    {
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_surfaceGetOpacity(t_ilm_surface surfaceId, t_ilm_float *pOpacity)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("GetSurfaceOpacity");
    if (pOpacity
        && command
        && gIpcModule.appendUint(command, surfaceId)
        && sendAndWaitForResponse(command, &response, gResponseTimeout)
        && gIpcModule.getDouble(response, pOpacity))
    {
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_surfaceSetSourceRectangle(t_ilm_surface surfaceId, t_ilm_int x, t_ilm_int y, t_ilm_int width, t_ilm_int height)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("SetSurfaceSourceRegion");
    if (command
        && gIpcModule.appendUint(command, surfaceId)
        && gIpcModule.appendUint(command, x)
        && gIpcModule.appendUint(command, y)
        && gIpcModule.appendUint(command, width)
        && gIpcModule.appendUint(command, height)
        && sendAndWaitForResponse(command, &response, gResponseTimeout))
    {
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_surfaceSetDestinationRectangle(t_ilm_surface surfaceId, t_ilm_int x, t_ilm_int y, t_ilm_int width, t_ilm_int height)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("SetSurfaceDestinationRegion");
    if (command
        && gIpcModule.appendUint(command, surfaceId)
        && gIpcModule.appendUint(command, x)
        && gIpcModule.appendUint(command, y)
        && gIpcModule.appendUint(command, width)
        && gIpcModule.appendUint(command, height)
        && sendAndWaitForResponse(command, &response, gResponseTimeout))
    {
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_surfaceGetDimension(t_ilm_surface surfaceId, t_ilm_uint *pDimension)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("GetSurfaceDimension");
    if (pDimension
        && command
        && gIpcModule.appendUint(command, surfaceId)
        && sendAndWaitForResponse(command, &response, gResponseTimeout)
        && gIpcModule.getUint(response, &pDimension[0])
        && gIpcModule.getUint(response, &pDimension[1]))
    {
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_surfaceSetDimension(t_ilm_surface surfaceId, t_ilm_uint *pDimension)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("SetSurfaceDimension");
    if (pDimension
        && command
        && gIpcModule.appendUint(command, surfaceId)
        && gIpcModule.appendUint(command, pDimension[0])
        && gIpcModule.appendUint(command, pDimension[1])
        && sendAndWaitForResponse(command, &response, gResponseTimeout))
    {
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_surfaceGetPosition(t_ilm_surface surfaceId, t_ilm_uint *pPosition)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("GetSurfacePosition");
    if (pPosition
        && command
        && gIpcModule.appendUint(command, surfaceId)
        && sendAndWaitForResponse(command, &response, gResponseTimeout)
        && gIpcModule.getUint(response, &pPosition[0])
        && gIpcModule.getUint(response, &pPosition[1]))
    {
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_surfaceSetPosition(t_ilm_surface surfaceId, t_ilm_uint *pPosition)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("SetSurfacePosition");
    if (pPosition
        && command
        && gIpcModule.appendUint(command, surfaceId)
        && gIpcModule.appendUint(command, pPosition[0])
        && gIpcModule.appendUint(command, pPosition[1])
        && sendAndWaitForResponse(command, &response, gResponseTimeout))
    {
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_surfaceSetOrientation(t_ilm_surface surfaceId, ilmOrientation orientation)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("SetSurfaceOrientation");
    if (command
        && gIpcModule.appendUint(command, surfaceId)
        && gIpcModule.appendUint(command, orientation)
        && sendAndWaitForResponse(command, &response, gResponseTimeout))
    {
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_surfaceGetOrientation(t_ilm_surface surfaceId, ilmOrientation *pOrientation)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("GetSurfaceOrientation");
    if (pOrientation
        && command
        && gIpcModule.appendUint(command, surfaceId)
        && sendAndWaitForResponse(command, &response, gResponseTimeout)
        && gIpcModule.getUint(response, pOrientation))
    {
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_surfaceGetPixelformat(t_ilm_layer surfaceId, ilmPixelFormat *pPixelformat)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("GetSurfacePixelformat");
    if (pPixelformat
        && command
        && gIpcModule.appendUint(command, surfaceId)
        && sendAndWaitForResponse(command, &response, gResponseTimeout)
        && gIpcModule.getUint(response, pPixelformat))
    {
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_surfaceSetChromaKey(t_ilm_surface surfaceId, t_ilm_int* pColor)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("SetSurfaceChromaKey");
    if (command
        && gIpcModule.appendUint(command, surfaceId))
    {
        t_ilm_bool comResult = ILM_TRUE;

        // Checking pColor has a content, otherwise chromakey is disabled
        if (pColor)
        {
            const t_ilm_uint number = 3;
            comResult = gIpcModule.appendUintArray(command, (t_ilm_uint *)pColor, number);
        }
        if (comResult
            && sendAndWaitForResponse(command, &response, gResponseTimeout))
        {
            returnValue = ILM_SUCCESS;
        }
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_displaySetRenderOrder(t_ilm_display display, t_ilm_layer *pLayerId, const t_ilm_uint number)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("SetRenderOrderOfLayers");
    if (pLayerId
        && command
        && gIpcModule.appendUintArray(command, pLayerId, number)
        && gIpcModule.appendUint(command, display)
        && sendAndWaitForResponse(command, &response, gResponseTimeout))
    {
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_getScreenIDs(t_ilm_uint* pNumberOfIDs, t_ilm_uint** ppIDs)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("GetScreenIDs");
    if (pNumberOfIDs && ppIDs
        && command
        && sendAndWaitForResponse(command, &response, gResponseTimeout)
        && gIpcModule.getUintArray(response, ppIDs, (t_ilm_int *)pNumberOfIDs))
    {
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_takeScreenshot(t_ilm_uint screen, t_ilm_const_string filename)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("ScreenShot");
    if (command
        && gIpcModule.appendUint(command, screen)
        && gIpcModule.appendString(command, filename)
        && sendAndWaitForResponse(command, &response, gResponseTimeout))
    {
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_takeLayerScreenshot(t_ilm_const_string filename, t_ilm_layer layerid)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("ScreenShotOfLayer");
    if (command
        && gIpcModule.appendString(command, filename)
        && gIpcModule.appendUint(command, layerid)
        && sendAndWaitForResponse(command, &response, gResponseTimeout))
    {
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_takeSurfaceScreenshot(t_ilm_const_string filename, t_ilm_surface surfaceid)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("ScreenShotOfSurface");
    if (command
        && gIpcModule.appendString(command, filename)
        && gIpcModule.appendUint(command, surfaceid)
        && sendAndWaitForResponse(command, &response, gResponseTimeout))
    {
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_SetKeyboardFocusOn(t_ilm_surface surfaceId)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("SetKeyboardFocusOn");
    if (command
        && gIpcModule.appendUint(command, surfaceId)
        && sendAndWaitForResponse(command, &response, gResponseTimeout))
    {
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_GetKeyboardFocusSurfaceId(t_ilm_surface* pSurfaceId)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("GetKeyboardFocusSurfaceId");
    if (command
        && sendAndWaitForResponse(command, &response, gResponseTimeout)
        && gIpcModule.getUint(response, pSurfaceId))
    {
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_UpdateInputEventAcceptanceOn(t_ilm_surface surfaceId, ilmInputDevice devices, t_ilm_bool acceptance)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("UpdateInputEventAcceptanceOn");
    if (command
        && gIpcModule.appendUint(command, surfaceId)
        && gIpcModule.appendUint(command, devices)
        && gIpcModule.appendBool(command, acceptance)
        && sendAndWaitForResponse(command, &response, gResponseTimeout))
    {
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_SetOptimizationMode(ilmOptimization id, ilmOptimizationMode mode)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("SetOptimizationMode");
    if (command
        && gIpcModule.appendUint(command,id)
        && gIpcModule.appendUint(command,mode)
        && sendAndWaitForResponse(command, &response, gResponseTimeout))
    {
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_GetOptimizationMode(ilmOptimization id, ilmOptimizationMode* pMode)
{
    ilmErrorTypes returnValue = ILM_FAILED;
    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("GetOptimizationMode");
    if (command
        && gIpcModule.appendUint(command,id)
        && sendAndWaitForResponse(command, &response, gResponseTimeout)
        && gIpcModule.getUint(response, pMode))
    {
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_commitChanges()
{
    ilmErrorTypes returnValue = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("CommitChanges");

    if (command
        && sendAndWaitForResponse(command, &response, gResponseTimeout))
    {
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_layerAddNotification(t_ilm_layer layer, layerNotificationFunc callback)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    if (findLayerCallback(layer))
    {
        return ILM_ERROR_INVALID_ARGUMENTS;
    }

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("LayerAddNotification");
    if (command
        && gIpcModule.appendUint(command, layer)
        && sendAndWaitForResponse(command, &response, gResponseTimeout))
    {
        addLayerCallback(layer, callback);
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_layerRemoveNotification(t_ilm_layer layer)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    if (!findLayerCallback(layer))
    {
        return ILM_ERROR_INVALID_ARGUMENTS;
    }

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("LayerRemoveNotification");
    if (command
        && gIpcModule.appendUint(command, layer)
        && sendAndWaitForResponse(command, &response, gResponseTimeout))
    {
        removeLayerCallback(layer);
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_surfaceAddNotification(t_ilm_surface surface, surfaceNotificationFunc callback)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    if (findSurfaceCallback(surface))
    {
        return ILM_ERROR_INVALID_ARGUMENTS;
    }

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("SurfaceAddNotification");
    if (command
        && gIpcModule.appendUint(command, surface)
        && sendAndWaitForResponse(command, &response, gResponseTimeout))
    {
        addSurfaceCallback(surface, callback);
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_surfaceRemoveNotification(t_ilm_surface surface)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    if (!findSurfaceCallback(surface))
    {
        return ILM_ERROR_INVALID_ARGUMENTS;
    }

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("SurfaceRemoveNotification");
    if (command
        && gIpcModule.appendUint(command, surface)
        && sendAndWaitForResponse(command, &response, gResponseTimeout))
    {
        removeSurfaceCallback(surface);
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}

ilmErrorTypes ilm_getPropertiesOfScreen(t_ilm_display screenID, struct ilmScreenProperties* pScreenProperties)
{
    ilmErrorTypes returnValue = ILM_FAILED;

    t_ilm_message response = 0;
    t_ilm_message command = gIpcModule.createMessage("GetPropertiesOfScreen");
    if (pScreenProperties
        && command
        && gIpcModule.appendUint(command, screenID)
        && sendAndWaitForResponse(command, &response, gResponseTimeout)
        && gIpcModule.getUintArray(response, &pScreenProperties->layerIds, &pScreenProperties->layerCount)
        && gIpcModule.getUint(response, &pScreenProperties->harwareLayerCount)
        && gIpcModule.getUint(response, &pScreenProperties->screenWidth)
        && gIpcModule.getUint(response, &pScreenProperties->screenHeight))
    {
        returnValue = ILM_SUCCESS;
    }
    gIpcModule.destroyMessage(response);
    gIpcModule.destroyMessage(command);
    return returnValue;
}
