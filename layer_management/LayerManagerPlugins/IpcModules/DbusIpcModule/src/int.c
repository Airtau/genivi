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


t_ilm_bool appendInt(t_ilm_message message, const t_ilm_int value)
{
    dbusmessage* msg = (dbusmessage*)message;
    return dbus_message_iter_append_basic(&msg->iter, DBUS_TYPE_INT32, &value);
}

t_ilm_bool appendIntArray(t_ilm_message message, const int* valueArray, int arraySize)
{
    dbusmessage* msg = (dbusmessage*)message;
    t_ilm_bool returnValue = ILM_FALSE;
    char signature[2] = { DBUS_TYPE_UINT32, 0 };
    DBusMessageIter arrayIter;

    returnValue = dbus_message_iter_open_container(&msg->iter, DBUS_TYPE_ARRAY, signature, &arrayIter);

    t_ilm_int index = 0;
    for (index = 0; index < arraySize; ++index)
    {
        returnValue &= dbus_message_iter_append_basic(&arrayIter, DBUS_TYPE_INT32, &valueArray[index]);
    }

    returnValue &= dbus_message_iter_close_container(&msg->iter, &arrayIter);

    return returnValue;
}

t_ilm_bool getIntArray(t_ilm_message message, t_ilm_int** valueArray, t_ilm_int* arraySize)
{
    dbusmessage* msg = (dbusmessage*)message;
    t_ilm_bool returnValue = ILM_FALSE;

    t_ilm_int type = dbus_message_iter_get_arg_type(&msg->iter);

    if (DBUS_TYPE_ARRAY == type)
    {
        returnValue = ILM_TRUE;

        DBusMessageIter arrayIter;
        int index = 0;

        dbus_message_iter_recurse(&msg->iter, &arrayIter);

        while (DBUS_TYPE_INVALID != dbus_message_iter_get_arg_type(&arrayIter))
        {
            if ((*arraySize > index))
            {
                dbus_message_iter_get_basic(&arrayIter, &valueArray[index]);
            }
            else
            {
                returnValue = ILM_FALSE;
            }
            dbus_message_iter_next(&arrayIter);
            index++;
        }
        *arraySize = index;
    }
    else
    {
        printf("ERROR: expected: DBUS_TYPE_ARRAY, received ");
        printTypeName(type);
    }

    return returnValue;
}

t_ilm_bool getInt(t_ilm_message message, t_ilm_int* value)
{
    dbusmessage* msg = (dbusmessage*)message;
    t_ilm_bool returnValue = ILM_FALSE;

    int type = dbus_message_iter_get_arg_type(&msg->iter);

    if (DBUS_TYPE_INT32 == type)
    {
        dbus_message_iter_get_basic(&msg->iter, value);
        dbus_message_iter_next(&msg->iter);
        returnValue = ILM_TRUE;
    }
    else
    {
        printf("ERROR: expected: DBUS_TYPE_INT32, received ");
        printTypeName(type);
    }

    return returnValue;
}

