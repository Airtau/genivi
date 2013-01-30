/***************************************************************************
 *
 * Copyright 2012 BMW Car IT GmbH
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
#include "IpcModule.h"
#include "common.h"
#include "DBUSConfiguration.h"
#include "introspection.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // strdup
#include <errno.h>

//=============================================================================
// prototypes internal functions
//=============================================================================
void handleWatchesForFds(fd_set in, fd_set out);
t_ilm_bool dispatchIncomingMessages();

void registerSignalForNotification(dbusmessage* message, char* signalName);
void unregisterSignalForNotification(dbusmessage* message, char* signalName);

//=============================================================================
// message handling
//=============================================================================
t_ilm_message createMessage(t_ilm_const_string name)
{
    if (!gDbus.initialized)
    {
        return 0;
    }

    dbusmessage* newMessage = (dbusmessage*)malloc(sizeof(dbusmessage));
    newMessage->type = IpcMessageTypeCommand;
    newMessage->dbusNativeType = DBUS_MESSAGE_TYPE_METHOD_CALL;
    pthread_mutex_lock(&gDbus.mutex);
    newMessage->pMessage = dbus_message_new_method_call(ILM_SERVICE_NAME,
                                                        ILM_PATH_COMPOSITE_SERVICE,
                                                        ILM_INTERFACE_COMPOSITE_SERVICE,
                                                        name);
    pthread_mutex_unlock(&gDbus.mutex);
    dbus_message_iter_init_append(newMessage->pMessage, &newMessage->iter);
    return (t_ilm_message)newMessage;
}

t_ilm_message createResponse(t_ilm_message message)
{
    if (!gDbus.initialized)
    {
        return NULL;
    }
    dbusmessage* receivedMessage = (dbusmessage*)message;
    dbusmessage* newResponse = (dbusmessage*)malloc(sizeof(dbusmessage));

    newResponse->type = IpcMessageTypeCommand;
    newResponse->dbusNativeType = DBUS_MESSAGE_TYPE_METHOD_RETURN;

    pthread_mutex_lock(&gDbus.mutex);
    newResponse->pMessage = dbus_message_new(DBUS_MESSAGE_TYPE_METHOD_RETURN);
    pthread_mutex_unlock(&gDbus.mutex);

    const char* member = dbus_message_get_member(receivedMessage->pMessage);
    const char* destination = dbus_message_get_sender(receivedMessage->pMessage);
    dbus_message_set_member(newResponse->pMessage, member);
    dbus_message_set_destination(newResponse->pMessage, destination);
    dbus_message_set_reply_serial(newResponse->pMessage, receivedMessage->dbusSerial);
    dbus_message_iter_init_append(newResponse->pMessage, &newResponse->iter);

    return (t_ilm_message)newResponse;
}

t_ilm_message createErrorResponse(t_ilm_message message)
{
    if (!gDbus.initialized)
    {
        return 0;
    }

    dbusmessage* receivedMessage = (dbusmessage*)message;
    dbusmessage* newResponse = (dbusmessage*)malloc(sizeof(dbusmessage));
    memset(newResponse, 0, sizeof(dbusmessage));

    newResponse->type = IpcMessageTypeError;
    newResponse->dbusNativeType = DBUS_MESSAGE_TYPE_ERROR;

    pthread_mutex_lock(&gDbus.mutex);
    newResponse->pMessage = dbus_message_new(DBUS_MESSAGE_TYPE_ERROR);
    pthread_mutex_unlock(&gDbus.mutex);

    const char* member = dbus_message_get_member(receivedMessage->pMessage);
    const char* destination = dbus_message_get_sender(receivedMessage->pMessage);
    dbus_message_set_member(newResponse->pMessage, member);
    dbus_message_set_destination(newResponse->pMessage, destination);
    dbus_message_set_error_name(newResponse->pMessage, DBUS_SERVICE_ERROR);
    dbus_message_set_reply_serial(newResponse->pMessage, receivedMessage->dbusSerial);
    dbus_message_iter_init_append(newResponse->pMessage, &newResponse->iter);

    return (t_ilm_message)newResponse;
}

t_ilm_message createNotification(t_ilm_const_string name)
{
    if (!gDbus.initialized)
    {
        return 0;
    }

    dbusmessage* newNotification = (dbusmessage*)malloc(sizeof(dbusmessage));

    newNotification->type = IpcMessageTypeNotification;
    newNotification->dbusNativeType = DBUS_MESSAGE_TYPE_SIGNAL;

    if (!gDbus.isClient)
    {
        pthread_mutex_lock(&gDbus.mutex);
        newNotification->pMessage = dbus_message_new_signal(ILM_PATH_COMPOSITE_SERVICE,
                                                            ILM_INTERFACE_COMPOSITE_SERVICE,
                                                            name);
        pthread_mutex_unlock(&gDbus.mutex);
        dbus_message_iter_init_append(newNotification->pMessage, &newNotification->iter);
    }

    return (t_ilm_message)newNotification;
}

t_ilm_bool sendToClients(t_ilm_message message, t_ilm_client_handle* receiverList, int receiverCount)
{
    (void)receiverList;
    (void)receiverCount;

    if (gDbus.isClient)
    {
        return ILM_FALSE;
    }

    dbusmessage* messageToSend = (dbusmessage*)message;
    t_ilm_uint serial = dbus_message_get_serial(messageToSend->pMessage);

    dbus_message_set_path(messageToSend->pMessage, ILM_PATH_COMPOSITE_SERVICE);
    dbus_message_set_interface(messageToSend->pMessage, ILM_INTERFACE_COMPOSITE_SERVICE);
    pthread_mutex_lock(&gDbus.mutex);
    dbus_bool_t success = dbus_connection_send(gDbus.connection, messageToSend->pMessage, &serial);
    pthread_mutex_unlock(&gDbus.mutex);
    if (!success)
    {
        printf("DBUSIpcModule: Out Of Memory!\n");
        exit(1);
    }

    return ILM_TRUE;
}

t_ilm_bool sendToService(t_ilm_message message)
{
    if (!gDbus.isClient)
    {
        return ILM_FALSE;
    }

    dbusmessage* messageToSend = (dbusmessage*)message;
    t_ilm_uint serial = 1;
    pthread_mutex_lock(&gDbus.mutex);
    dbus_bool_t success = dbus_connection_send(gDbus.connection, messageToSend->pMessage, &serial);
    pthread_mutex_unlock(&gDbus.mutex);
    if (!success)
    {
        printf("DBUSIpcModule: Out Of Memory!\n");
        exit(1);
    }

    const char* msgName = dbus_message_get_member(messageToSend->pMessage);

    if (0 == strcmp(msgName, "LayerAddNotification"))
    {
        registerSignalForNotification(messageToSend, "NotificationForLayer");
    }

    if (0 == strcmp(msgName, "LayerRemoveNotification"))
    {
        unregisterSignalForNotification(messageToSend, "NotificationForLayer");
    }

    if (0 == strcmp(msgName, "SurfaceAddNotification"))
    {
        registerSignalForNotification(messageToSend, "NotificationForSurface");
    }

    if (0 == strcmp(msgName, "SurfaceRemoveNotification"))
    {
        unregisterSignalForNotification(messageToSend, "NotificationForSurface");
    }

    return ILM_TRUE;
}

t_ilm_message receive(t_ilm_int timeoutInMs)
{
    gpIncomingMessage = (dbusmessage*)malloc(sizeof(dbusmessage));
    memset(gpIncomingMessage, 0, sizeof(dbusmessage));

    if (!gDbus.initialized)
    {
        return (t_ilm_message)gpIncomingMessage;
    }

    if (dispatchIncomingMessages())
    {
        return (t_ilm_message)gpIncomingMessage;
    }

    fd_set readFds = gDbus.incomingSockets;
    fd_set writeFds = gDbus.outgoingSockets;
    int fdMax = (gDbus.incomingSocketsMax > gDbus.outgoingSocketsMax) ?
                 gDbus.incomingSocketsMax : gDbus.outgoingSocketsMax;

    int fdsReady = 0;

    if (timeoutInMs < 0)
    {
        fdsReady = select(fdMax, &readFds, &writeFds, 0, NULL);
    }
    else
    {
        struct timeval timeoutValue;
        timeoutValue.tv_sec = timeoutInMs / 1000;
        timeoutValue.tv_usec = (timeoutInMs % 1000) * 1000;
        fdsReady = select(fdMax, &readFds, &writeFds, 0, &timeoutValue);
    }

    switch (fdsReady)
    {
    case -1: // error: select was cancelled -> shutdown
        gpIncomingMessage->type = IpcMessageTypeShutdown;
        break;

    case 0: // timeout
        gpIncomingMessage->type = IpcMessageTypeNone;
        break;

    default: // message or shutdown
        handleWatchesForFds(readFds, writeFds);
        dispatchIncomingMessages();
        break;
    }

    return (t_ilm_message)gpIncomingMessage;
}

t_ilm_const_string getMessageName(t_ilm_message message)
{
    dbusmessage* msg = (dbusmessage*)message;
    return msg->pMessage ? dbus_message_get_member(msg->pMessage) : NULL;
}

t_ilm_message_type getMessageType(t_ilm_message message)
{
    dbusmessage* msg = (dbusmessage*)message;
    return msg->type;
}

t_ilm_const_string getSenderName(t_ilm_message message)
{
    dbusmessage* msg = (dbusmessage*)message;
    return msg->pMessage ? dbus_message_get_sender(msg->pMessage) : NULL;
}

t_ilm_client_handle getSenderHandle(t_ilm_message message)
{
    t_ilm_uint result = 0;

    const char* sender = getSenderName(message);
    if (sender)
    {
        float f = atof(&sender[1]);
        result = (t_ilm_uint)(f * 1000000);
    };
    return (t_ilm_client_handle)result;
}

t_ilm_bool destroyMessage(t_ilm_message message)
{
    dbusmessage* msg = (dbusmessage*)message;
    if (msg->pMessage)
    {
        pthread_mutex_lock(&gDbus.mutex);
        dbus_message_unref(msg->pMessage);
        pthread_mutex_unlock(&gDbus.mutex);
    }
    free(msg);
    return ILM_TRUE;
}

//=============================================================================
// prototypes internal functions
//=============================================================================
void handleWatchesForFds(fd_set in, fd_set out)
{
    int fd = 0;

    for (fd = 0; fd < gDbus.incomingSocketsMax; ++fd)
    {
        if (FD_ISSET(fd, &in))
        {
            DBusWatch* activeWatch = gDbus.incomingWatch[fd];

            if (activeWatch)
            {
                pthread_mutex_lock(&gDbus.mutex);
                dbus_bool_t success = dbus_watch_handle(activeWatch, DBUS_WATCH_READABLE);
                pthread_mutex_unlock(&gDbus.mutex);

                if (!success)
                {
                    printf("incoming dbus_watch_handle() failed\n");
                }
            }
            else
            {
                printf("no watch was found for incoming fd %d, not calling dbus_watch_handle(NULL)\n", fd);
            }
        }
    }

    for (fd = 0; fd < gDbus.outgoingSocketsMax; ++fd)
    {
        if (FD_ISSET(fd, &out))
        {
            DBusWatch* activeWatch = gDbus.outgoingWatch[fd];

            if (activeWatch)
            {
                pthread_mutex_lock(&gDbus.mutex);
                dbus_bool_t success = dbus_watch_handle(activeWatch, DBUS_WATCH_WRITABLE);
                pthread_mutex_unlock(&gDbus.mutex);

                if (!success)
                {
                    printf("outgoing dbus_watch_handle() failed\n");
                }
            }
            else
            {
                printf("no watch was found for outgoing fd %d, not calling dbus_watch_handle(NULL)\n", fd);
            }
        }
    }
}

t_ilm_bool dispatchIncomingMessages()
{
    t_ilm_bool dispatched = ILM_FALSE;
    if (DBUS_DISPATCH_DATA_REMAINS == dbus_connection_get_dispatch_status(gDbus.connection))
    {
        pthread_mutex_lock(&gDbus.mutex);
        dbus_connection_dispatch(gDbus.connection);
        pthread_mutex_unlock(&gDbus.mutex);
        dispatched = ILM_TRUE;
    }
    return dispatched;
}

void registerSignalForNotification(dbusmessage* message, char* signalName)
{
    char rule[1024];
    t_ilm_uint id;
    dbus_message_get_args(message->pMessage, NULL,
                          DBUS_TYPE_UINT32, &id,
                          DBUS_TYPE_INVALID);

    sprintf(rule,
            "type='signal',sender='%s',interface='%s',member='%s%u'",
            ILM_INTERFACE_COMPOSITE_SERVICE,
            ILM_INTERFACE_COMPOSITE_SERVICE,
            signalName,
            id);

    // do not block here, race condition with receive thread.
    // according to dbus documentation almost impossible to fail
    // (only if out of memory)
    // if result is important, create method call manually
    // and use main loop for communication
    pthread_mutex_lock(&gDbus.mutex);
    dbus_bus_add_match(gDbus.connection, rule, NULL);
    pthread_mutex_unlock(&gDbus.mutex);
}

void unregisterSignalForNotification(dbusmessage* message, char* signalName)
{
    char rule[1024];
    t_ilm_uint id;
    dbus_message_get_args(message->pMessage, NULL,
                          DBUS_TYPE_UINT32, &id,
                          DBUS_TYPE_INVALID);

    printf("DbusIpcModule: removing signal handler %s%d\n", signalName, id);
    sprintf(rule,
            "type='signal',sender='%s',interface='%s',member='%s%d'",
            ILM_INTERFACE_COMPOSITE_SERVICE,
            ILM_INTERFACE_COMPOSITE_SERVICE,
            signalName,
            id);

    // do not block here, race condition with receive thread.
    // according to dbus documentation almost impossible to fail
    // (only if out of memory)
    // if result is important, create method call manually
    // and use main loop for communication
    pthread_mutex_lock(&gDbus.mutex);
    dbus_bus_remove_match(gDbus.connection, rule, NULL);
    pthread_mutex_unlock(&gDbus.mutex);
}



//=============================================================================
// print debug information
//=============================================================================
void printTypeName(int type)
{
    switch (type)
    {
    case DBUS_TYPE_ARRAY:
        printf("DBUS_TYPE_ARRAY\n");
        break;
    case DBUS_TYPE_BOOLEAN:
        printf("DBUS_TYPE_BOOLEAN\n");
        break;
    case DBUS_TYPE_DOUBLE:
        printf("DBUS_TYPE_DOUBLE\n");
        break;
    case DBUS_TYPE_INT32:
        printf("DBUS_TYPE_INT32\n");
        break;
    case DBUS_TYPE_STRING:
        printf("DBUS_TYPE_STRING\n");
        break;
    case DBUS_TYPE_UINT32:
        printf("DBUS_TYPE_UINT\n");
        break;
    default:
        printf("unknown (%s)\n", (char*)&type);
        break;
    }
}

