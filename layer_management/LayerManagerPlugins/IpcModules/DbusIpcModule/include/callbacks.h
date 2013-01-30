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

#ifndef _CALLBACKS_H_
#define _CALLBACKS_H_

#include <dbus/dbus.h>


// message filters

DBusHandlerResult filterLogging(DBusConnection *connection, DBusMessage *message, void *data);
DBusHandlerResult filterNameAcquired(DBusConnection *connection, DBusMessage *message, void *data);
DBusHandlerResult filterNameOwnerChanged(DBusConnection *connection, DBusMessage *message, void *data);
DBusHandlerResult filterIntrospection(DBusConnection *connection, DBusMessage *message, void *data);
DBusHandlerResult filterLayerManagerCommands(DBusConnection *connection, DBusMessage *message, void *data);
DBusHandlerResult filterLayerManagerErrors(DBusConnection *connection, DBusMessage *message, void *data);
DBusHandlerResult filterLayerManagerNotifications(DBusConnection *connection, DBusMessage *message, void *data);
DBusHandlerResult filterDiscardUnexpected(DBusConnection *connection, DBusMessage *message, void *data);


// watches

dbus_bool_t addWatch(DBusWatch* watch, void* data);
void removeWatch(DBusWatch* watch, void* data);
void toggleWatch(DBusWatch* watch, void* data);


#endif // _CALLBACKS_H_
