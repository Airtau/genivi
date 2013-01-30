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

//-----------------------------------------------------------------------------
// append simple data types
//-----------------------------------------------------------------------------

t_ilm_bool appendGenericValue(struct SocketMessage* msg, const char protocolType, const char size, const void* value)
{
    // size check: is message size reached
    if (sizeof(msg->paket) - sizeof(msg->paket.data)  // header
        + msg->index + size                           // + data
        > SOCKET_MAX_MESSAGE_SIZE)
    {
        printf("Error: max message size exceeded.\n");
        return ILM_FALSE;
    }

    // append protocol type
    msg->paket.data[msg->index] = protocolType;
    msg->index += sizeof(protocolType);

    // append size of data
    msg->paket.data[msg->index] = size;
    msg->index += sizeof(size);

    // append data
    memcpy(&msg->paket.data[msg->index], value, size);
    msg->index += size;

    return ILM_TRUE;
}

t_ilm_bool appendUint(t_ilm_message message, const t_ilm_uint value)
{
    struct SocketMessage* msg = (struct SocketMessage*)message;
    return appendGenericValue(msg, SOCKET_MESSAGE_TYPE_UINT, sizeof(t_ilm_uint), &value);
}

t_ilm_bool appendInt(t_ilm_message message, const t_ilm_int value)
{
    struct SocketMessage* msg = (struct SocketMessage*)message;
    return appendGenericValue(msg, SOCKET_MESSAGE_TYPE_INT, sizeof(t_ilm_int), &value);
}

t_ilm_bool appendBool(t_ilm_message message, const t_ilm_bool value)
{
    struct SocketMessage* msg = (struct SocketMessage*)message;
    return appendGenericValue(msg, SOCKET_MESSAGE_TYPE_BOOL, sizeof(t_ilm_bool), &value);
}

t_ilm_bool appendDouble(t_ilm_message message, const t_ilm_float value)
{
    struct SocketMessage* msg = (struct SocketMessage*)message;
    return appendGenericValue(msg, SOCKET_MESSAGE_TYPE_DOUBLE, sizeof(t_ilm_float), &value);
}

t_ilm_bool appendString(t_ilm_message message, t_ilm_const_string value)
{
    struct SocketMessage* msg = (struct SocketMessage*)message;
    return appendGenericValue(msg, SOCKET_MESSAGE_TYPE_STRING, strlen(value), value);
}

//-----------------------------------------------------------------------------
// append array data types
//-----------------------------------------------------------------------------

t_ilm_bool appendGenericArray(struct SocketMessage* msg, const char arraySize, const char protocolType, const char size, const void* value)
{
    t_ilm_bool result = ILM_TRUE;

    // TODO: size check: is message size reached?

    // append array type
    msg->paket.data[msg->index] = SOCKET_MESSAGE_TYPE_ARRAY;
    msg->index += sizeof(protocolType);

    // append size of array
    msg->paket.data[msg->index] = arraySize;
    msg->index += sizeof(arraySize);

    // append data for each array entry
    char i = 0;
    for (i = 0; i < arraySize; ++i)
    {
        result &= appendGenericValue(msg, protocolType, size, (void*)((unsigned int)value + i * size));
    }

    return result;
}

t_ilm_bool appendUintArray(t_ilm_message message, const t_ilm_uint* valueArray, t_ilm_int arraySize)
{
    struct SocketMessage* msg = (struct SocketMessage*)message;
    return appendGenericArray(msg, arraySize, SOCKET_MESSAGE_TYPE_UINT, sizeof(t_ilm_uint), valueArray);
}

t_ilm_bool appendIntArray(t_ilm_message message, const t_ilm_int* valueArray, t_ilm_int arraySize)
{
    struct SocketMessage* msg = (struct SocketMessage*)message;
    return appendGenericArray(msg, arraySize, SOCKET_MESSAGE_TYPE_INT, sizeof(t_ilm_int), valueArray);
}

t_ilm_bool appendBoolArray(t_ilm_message message, const t_ilm_bool* valueArray, t_ilm_int arraySize)
{
    struct SocketMessage* msg = (struct SocketMessage*)message;
    return appendGenericArray(msg, arraySize, SOCKET_MESSAGE_TYPE_BOOL, sizeof(t_ilm_bool), valueArray);
}

t_ilm_bool appendDoubleArray(t_ilm_message message, const t_ilm_float* valueArray, t_ilm_int arraySize)
{
    struct SocketMessage* msg = (struct SocketMessage*)message;
    return appendGenericArray(msg, arraySize, SOCKET_MESSAGE_TYPE_DOUBLE, sizeof(t_ilm_float), valueArray);
}

// TODO appendStringArray()
