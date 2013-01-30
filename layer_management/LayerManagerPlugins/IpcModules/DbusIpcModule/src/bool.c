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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  // memcpy


t_ilm_bool appendBool(t_ilm_message message, const t_ilm_bool value)
{
    dbusmessage* msg = (dbusmessage*)message;
    return dbus_message_iter_append_basic(&msg->iter, DBUS_TYPE_BOOLEAN, &value);
}

t_ilm_bool getBool(t_ilm_message message, t_ilm_bool* value)
{
    t_ilm_bool returnValue = ILM_FALSE;
    dbusmessage* msg = (dbusmessage*)message;
    t_ilm_int type = dbus_message_iter_get_arg_type(&msg->iter);

    if (DBUS_TYPE_BOOLEAN == type)
    {
        dbus_message_iter_get_basic(&msg->iter, value);
        dbus_message_iter_next(&msg->iter);
        returnValue = ILM_TRUE;
    }
    else
    {
        printf("ERROR: expected: DBUS_TYPE_BOOLEAN, received ");
        printTypeName(type);
    }

    return returnValue;
}
