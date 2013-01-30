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
#ifndef __COMMON_H__
#define __COMMON_H__

#include "ilm_types.h"
#include <dbus/dbus.h>
#include <sys/select.h>
#include <pthread.h>


//=============================================================================
// data structures
//=============================================================================
typedef struct
{
    t_ilm_message_type type;
    int dbusNativeType;
    int dbusSerial;
    DBusMessage* pMessage;
    DBusMessageIter iter;
} dbusmessage;

typedef struct
{
    t_ilm_bool initialized;
    t_ilm_bool isClient;

    pthread_mutex_t mutex;

    DBusConnection* connection;
    DBusBusType type;
    DBusError error;

    DBusWatch* incomingWatch[FD_SETSIZE];
    fd_set incomingSockets;
    int incomingSocketsMax;

    DBusWatch* outgoingWatch[FD_SETSIZE];
    fd_set outgoingSockets;
    int outgoingSocketsMax;
} dbus;

//=============================================================================
// global variables
//=============================================================================
dbus gDbus;
dbusmessage* gpIncomingMessage;

//=============================================================================
// prototypes
//=============================================================================
void printTypeName(int type);
void unregisterMessageFunc(DBusConnection* conn, void *user_data);
t_ilm_bool appendString(t_ilm_message message, const char* value);
t_ilm_bool getString(t_ilm_message message, char* value);
t_ilm_bool sendToClients(t_ilm_message message, t_ilm_client_handle* receiverList, int receiverCount);

//=============================================================================
// callback functions
//=============================================================================
void callbackUnregister(DBusConnection *connection, void *user_data);
DBusHandlerResult callbackMessage(DBusConnection *connection, DBusMessage *message, void *user_data);


#endif // __COMMON_H__
