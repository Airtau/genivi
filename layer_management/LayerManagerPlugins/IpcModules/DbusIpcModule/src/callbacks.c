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
#include "DBUSConfiguration.h"
#include "callbacks.h"
#include "common.h"
#include "introspection.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

t_ilm_message createResponse(t_ilm_message);

//=============================================================================
// message filters
//=============================================================================

DBusHandlerResult filterLayerManagerNotifications(DBusConnection *connection, DBusMessage *message, void *data)
{
    (void)connection;
    (void)data;

    const char* interfaceName = dbus_message_get_interface(message);
    if (!interfaceName)
    {
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }
    if (strcmp(ILM_INTERFACE_COMPOSITE_SERVICE, interfaceName)) // 0 == equals
    {
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }

    DBusHandlerResult result = DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    int messageType = dbus_message_get_type(message);

    if (gDbus.isClient && (messageType == DBUS_MESSAGE_TYPE_SIGNAL))
    {
        gpIncomingMessage->type = IpcMessageTypeNotification;
        gpIncomingMessage->dbusNativeType = messageType;
        gpIncomingMessage->dbusSerial = dbus_message_get_serial(message);
        gpIncomingMessage->pMessage = dbus_message_copy(message);

        dbus_message_iter_init(gpIncomingMessage->pMessage, &gpIncomingMessage->iter);

        result = DBUS_HANDLER_RESULT_HANDLED;
    }
    return result;
}

DBusHandlerResult filterLayerManagerCommands(DBusConnection *connection, DBusMessage *message, void *data)
{
    (void)connection;
    (void)data;

    const char* interfaceName = dbus_message_get_interface(message);
    if (!interfaceName)
    {
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }
    if (strcmp(ILM_INTERFACE_COMPOSITE_SERVICE, interfaceName)) // 0 == equals
    {
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }

    DBusHandlerResult result = DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    int messageType = dbus_message_get_type(message);

    if ((!gDbus.isClient && (messageType == DBUS_MESSAGE_TYPE_METHOD_CALL))
        || (gDbus.isClient && (messageType == DBUS_MESSAGE_TYPE_METHOD_RETURN)))
    {
        gpIncomingMessage->type = IpcMessageTypeCommand;
        gpIncomingMessage->dbusNativeType = messageType;
        gpIncomingMessage->dbusSerial = dbus_message_get_serial(message);
        gpIncomingMessage->pMessage = dbus_message_copy(message);

        dbus_message_iter_init(gpIncomingMessage->pMessage, &gpIncomingMessage->iter);

        result = DBUS_HANDLER_RESULT_HANDLED;
    }
    return result;
}

DBusHandlerResult filterLayerManagerErrors(DBusConnection *connection, DBusMessage *message, void *data)
{
    (void)connection;
    (void)data;

    const char* interfaceName = dbus_message_get_interface(message);
    if (!interfaceName)
    {
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }
    if (strcmp(ILM_INTERFACE_COMPOSITE_SERVICE, interfaceName)) // 0 == equals
    {
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }

    DBusHandlerResult result = DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    int messageType = dbus_message_get_type(message);

    if (messageType == DBUS_MESSAGE_TYPE_ERROR)
    {
        gpIncomingMessage->pMessage = dbus_message_copy(message);
        gpIncomingMessage->type = IpcMessageTypeError;
        gpIncomingMessage->dbusNativeType = messageType;

        dbus_message_iter_init(gpIncomingMessage->pMessage, &gpIncomingMessage->iter);

        char errorMsg[256];
        getString(gpIncomingMessage, errorMsg);
        printf("DbusIpcModule: LayerManagerService returned error: %s\n", errorMsg);

        result = DBUS_HANDLER_RESULT_HANDLED;
    }
    return result;
}

DBusHandlerResult filterLogging(DBusConnection *connection, DBusMessage *message, void *data)
{
    (void)connection;
    (void)data;

    printf("---------------------------------\n");
    printf("received:\n");
    printf("\tmember:    %s\n", dbus_message_get_member(message));
    printf("\ttype:      ");
    switch (dbus_message_get_type(message))
    {
    case DBUS_MESSAGE_TYPE_METHOD_CALL:
        printf("method call\n");
        break;
    case DBUS_MESSAGE_TYPE_METHOD_RETURN:
        printf("method return\n");
        break;
    case DBUS_MESSAGE_TYPE_ERROR:
        printf("error: %s\n", dbus_message_get_error_name(message));
        break;
    case DBUS_MESSAGE_TYPE_SIGNAL:
        printf("signal\n");
        break;
    default:
        printf("unknown\n");
        break;
    }
    printf("\tsender:    %s\n", dbus_message_get_sender(message));
    printf("\tdest:      %s\n", dbus_message_get_destination(message));
    printf("\tpath:      %s\n", dbus_message_get_path(message));
    printf("\tinterface: %s\n", dbus_message_get_interface(message));

    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

DBusHandlerResult filterNameAcquired(DBusConnection *connection, DBusMessage *message, void *data)
{
    (void)connection;
    (void)data;

    if (dbus_message_is_signal(message, DBUS_INTERFACE_DBUS, "NameAcquired"))
    {
        gpIncomingMessage->pMessage = dbus_message_copy(message);
        gpIncomingMessage->type = IpcMessageTypeNone;
    }

    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

DBusHandlerResult filterNameOwnerChanged(DBusConnection *connection, DBusMessage *message, void *data)
{
    (void)connection;
    (void)data;

    if (dbus_message_is_signal(message, DBUS_INTERFACE_DBUS, "NameOwnerChanged"))
    {
        char *name, *old, *new;
        if (!dbus_message_get_args(message, NULL,
                                  DBUS_TYPE_STRING, &name,
                                  DBUS_TYPE_STRING, &old,
                                  DBUS_TYPE_STRING, &new,
                                  DBUS_TYPE_INVALID))
        {
            printf("invalid NameOwnerChanged signal received\n");
            return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
        }

        if (new[0] == '\0')
        {
            gpIncomingMessage->type = IpcMessageTypeDisconnect;
            gpIncomingMessage->pMessage = dbus_message_copy(message);
            dbus_message_set_sender(gpIncomingMessage->pMessage, old);
        }

    }

    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

DBusHandlerResult filterIntrospection(DBusConnection *connection, DBusMessage *message, void *data)
{
    (void)connection;
    (void)data;

    if (dbus_message_is_method_call(message, DBUS_INTERFACE_INTROSPECTABLE, "Introspect"))
    {
        gpIncomingMessage->pMessage = dbus_message_copy(message);
        gpIncomingMessage->type = IpcMessageTypeNone; // none for client
        gpIncomingMessage->dbusNativeType = DBUS_MESSAGE_TYPE_METHOD_CALL;
        gpIncomingMessage->dbusSerial = dbus_message_get_serial(message);

        // generate introspection response content
        char introspectionString[65536];
        memset(introspectionString, 0, sizeof(introspectionString));
        generateIntrospectionString(introspectionString);

        // create and send introspection reply
        // note: we're in dispatch, so mutex is currently locked in this thread
        pthread_mutex_unlock(&gDbus.mutex);
        t_ilm_message introspectionResponse = createResponse(gpIncomingMessage);
        appendString(introspectionResponse, introspectionString);
        t_ilm_client_handle sender = (t_ilm_client_handle)dbus_message_get_sender(gpIncomingMessage->pMessage);
        sendToClients(introspectionResponse, &sender, 1);
        pthread_mutex_lock(&gDbus.mutex);

        return DBUS_HANDLER_RESULT_HANDLED;
    }

    return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

//=============================================================================
// watches
//=============================================================================

dbus_bool_t addWatch(DBusWatch* watch, void* data)
{
    (void)data;

    int sock = dbus_watch_get_unix_fd(watch);
    int flags = dbus_watch_get_flags(watch);

    if (flags & DBUS_WATCH_READABLE)
    {
        gDbus.incomingWatch[sock] = watch;

        if (dbus_watch_get_enabled(watch))
        {
            FD_SET(sock, &gDbus.incomingSockets);
            gDbus.incomingSocketsMax = (gDbus.incomingSocketsMax > sock) ? gDbus.incomingSocketsMax : sock;
            gDbus.incomingSocketsMax++;
        }
    }

    if (flags & DBUS_WATCH_WRITABLE)
    {
        gDbus.outgoingWatch[sock] = watch;

        if (dbus_watch_get_enabled(watch))
        {
            FD_SET(sock, &gDbus.outgoingSockets);
            gDbus.outgoingSocketsMax = (gDbus.outgoingSocketsMax > sock) ? gDbus.outgoingSocketsMax : sock;
            gDbus.outgoingSocketsMax++;
        }
    }

    return TRUE;
}

void removeWatch(DBusWatch* watch, void* data)
{
    (void)data;

    int sock = dbus_watch_get_unix_fd(watch);
    int flags = dbus_watch_get_flags(watch);

    if (flags & DBUS_WATCH_READABLE)
    {
        FD_CLR(sock, &gDbus.incomingSockets);
        gDbus.incomingWatch[sock] = NULL;
    }

    if (flags & DBUS_WATCH_WRITABLE)
    {
        FD_CLR(sock, &gDbus.outgoingSockets);
        gDbus.outgoingWatch[sock] = NULL;
    }
}

void toggleWatch(DBusWatch* watch, void* data)
{
    (void)data;

    if (dbus_watch_get_enabled(watch))
    {
        addWatch(watch, NULL);
    }
    else
    {
        removeWatch(watch, NULL);
    }
}
