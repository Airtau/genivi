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
#include "socketShared.h"
#include <stdio.h>
#include <stdlib.h> // malloc

//-----------------------------------------------------------------------------
// get simple data types
//-----------------------------------------------------------------------------

t_ilm_bool getGenericValue(struct SocketMessage* msg, void* value, const char protocolType, const char expectedSize)
{
    // get protocol value from message
    char readType = msg->paket.data[msg->index];
    msg->index += sizeof(readType);

    // if type mismatch, return to previous state, return with error
    if (readType != protocolType)
    {
        msg->index -= sizeof(readType);
        printf("command value type mismatch: expected '%s', got '%s'.\n",
               &protocolType, &readType);
        return ILM_FALSE;
    }

    // get size of value
    char size = msg->paket.data[msg->index];
    msg->index += sizeof(size);

    // if size mismatch, return to previous state, return with error
    // exception: strings have varying length
    if (protocolType != SOCKET_MESSAGE_TYPE_STRING
        && size != expectedSize)
    {
        msg->index -= sizeof(readType);
        msg->index -= sizeof(size);
        printf("command value size mismatch for type '%s': "
               "expected %d bytes, got %d bytes.\n",
               &protocolType, expectedSize, size);
        return ILM_FALSE;
    }

    // copy data to caller
    memcpy(value, &msg->paket.data[msg->index], size);
    msg->index += size;

    // if value is string, add end of string
    if (protocolType == SOCKET_MESSAGE_TYPE_STRING)
    {
        char* str = (char *)value + size;
        *str = '\0';
    }

    return ILM_TRUE;
}

t_ilm_bool getUint(t_ilm_message message, t_ilm_uint* value)
{
    struct SocketMessage* msg = (struct SocketMessage*)message;
    return getGenericValue(msg, value, SOCKET_MESSAGE_TYPE_UINT, sizeof(t_ilm_uint));
}

t_ilm_bool getInt(t_ilm_message message, t_ilm_int* value)
{
    struct SocketMessage* msg = (struct SocketMessage*)message;
    return getGenericValue(msg, value, SOCKET_MESSAGE_TYPE_INT, sizeof(t_ilm_int));
}

t_ilm_bool getBool(t_ilm_message message, t_ilm_bool* value)
{
    struct SocketMessage* msg = (struct SocketMessage*)message;
    return getGenericValue(msg, value, SOCKET_MESSAGE_TYPE_BOOL, sizeof(t_ilm_bool));
}

t_ilm_bool getDouble(t_ilm_message message, t_ilm_float* value)
{
    struct SocketMessage* msg = (struct SocketMessage*)message;
    return getGenericValue(msg, value, SOCKET_MESSAGE_TYPE_DOUBLE, sizeof(t_ilm_float));
}

t_ilm_bool getString(t_ilm_message message, char* value)
{
    struct SocketMessage* msg = (struct SocketMessage*)message;
    return getGenericValue(msg, value, SOCKET_MESSAGE_TYPE_STRING, sizeof(t_ilm_const_string));
}

//-----------------------------------------------------------------------------
// get array data types
//-----------------------------------------------------------------------------

t_ilm_bool getGenericArray(struct SocketMessage* msg, t_ilm_int* arraySize, void** value, const char protocolType, const char expectedSize)
{
    t_ilm_bool result = ILM_TRUE;

    // get protocol value from message
    char readType = msg->paket.data[msg->index];
    msg->index += sizeof(readType);

    // if type mismatch, return to previous state, return with error
    if (readType != SOCKET_MESSAGE_TYPE_ARRAY)
    {
        msg->index -= sizeof(readType);
        printf("command value type mismatch: expected '%c', got '%s'.\n",
               SOCKET_MESSAGE_TYPE_ARRAY, &readType);
        return ILM_FALSE;
    }

    // get size of array
    *arraySize = msg->paket.data[msg->index];
    msg->index += sizeof(msg->paket.data[msg->index]);

    // create array for result and set callers pointer
    *value = malloc(*arraySize * expectedSize);

    // get all values from array
    char i = 0;
    for (i = 0; i < *arraySize; ++i)
    {
        result &= getGenericValue(msg, ((*value) + expectedSize * i), protocolType, expectedSize);
    }

    return result;
}

t_ilm_bool getIntArray(t_ilm_message message, t_ilm_int** valueArray, t_ilm_int* arraySize)
{
    struct SocketMessage* msg = (struct SocketMessage*)message;
    return getGenericArray(msg, arraySize, (void**)valueArray, SOCKET_MESSAGE_TYPE_INT, sizeof(t_ilm_int));
}

t_ilm_bool getUintArray(t_ilm_message message, t_ilm_uint** valueArray, t_ilm_int* arraySize)
{
    struct SocketMessage* msg = (struct SocketMessage*)message;
    return getGenericArray(msg, arraySize, (void**)valueArray, SOCKET_MESSAGE_TYPE_UINT, sizeof(t_ilm_uint));
}

