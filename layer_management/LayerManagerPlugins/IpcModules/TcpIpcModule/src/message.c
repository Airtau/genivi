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
#include <errno.h>
#include <stdio.h>
#include <sys/select.h>
#include <stdlib.h>
#include <unistd.h>


//=============================================================================
// prototypes
//=============================================================================
t_ilm_bool acceptClientConnection();
t_ilm_bool sendToSocket(struct SocketMessage* msg, int socketNumber);
void receiveFromSocket(struct SocketMessage* msg, int socketNumber);

//=============================================================================
// incoming queue handling (one select may return more than one active
// descriptor, but receive must only return one message at a time.
// all messages are first received and added to this queue, so no
// messages get lost
//=============================================================================
struct QueueElement
{
    struct SocketMessage* data;
    struct QueueElement* next;
};

static struct QueueElement* oldest = NULL;
static struct QueueElement* latest = NULL;

void addToIncomingQueue(struct SocketMessage* data)
{
    struct QueueElement* newMessage = (struct QueueElement*)malloc(sizeof(struct QueueElement));
    newMessage->data = data;
    newMessage->next = NULL;
    if (!oldest)
    {
        oldest = latest = newMessage;
    }
    else
    {
        latest->next = newMessage;
        latest = latest->next;
    }
}

struct SocketMessage* getFromIncomingQueue()
{
    struct SocketMessage* data = NULL;
    if (oldest)
    {
        data = oldest->data;
        struct QueueElement* delPtr = oldest;
        oldest = oldest->next;
        free(delPtr);
    }
    return data;
}

//=============================================================================
// message handling
//=============================================================================
t_ilm_message createMessage(t_ilm_const_string name)
{
    struct SocketMessage* newMessage = (struct SocketMessage*)malloc(sizeof(struct SocketMessage));
    newMessage->paket.type = IpcMessageTypeCommand;
    newMessage->index = 0;
    appendString(newMessage, name);
    return (t_ilm_message)newMessage;
}

t_ilm_message createResponse(t_ilm_message receivedMessage)
{
    struct SocketMessage* newResponse = (struct SocketMessage*)malloc(sizeof(struct SocketMessage));
    newResponse->paket.type = IpcMessageTypeCommand;
    newResponse->index = 0;
    appendString(newResponse, getMessageName(receivedMessage));
    return (t_ilm_message)newResponse;
}

t_ilm_message createErrorResponse(t_ilm_message receivedMessage)
{
    struct SocketMessage* newErrorResponse = (struct SocketMessage*)malloc(sizeof(struct SocketMessage));
    newErrorResponse->paket.type = IpcMessageTypeError;
    newErrorResponse->index = 0;
    appendString(newErrorResponse, getMessageName(receivedMessage));
    return (t_ilm_message)newErrorResponse;
}

t_ilm_message createNotification(t_ilm_const_string name)
{
    struct SocketMessage* newNotification = (struct SocketMessage*)malloc(sizeof(struct SocketMessage));
    newNotification->paket.type = IpcMessageTypeNotification;
    newNotification->index = 0;
    appendString(newNotification, name);
    return (t_ilm_message)newNotification;
}

t_ilm_bool destroyMessage(t_ilm_message message)
{
    struct SocketMessage* msg = (struct SocketMessage*)message;
    if (msg)
    {
        free(msg);
    }
    return ILM_TRUE;
}

t_ilm_bool sendToClients(t_ilm_message message, t_ilm_client_handle* receiverList, int receiverCount)
{
    struct SocketMessage* msg = (struct SocketMessage*)message;
    if (gState.isClient)
    {
        return ILM_FALSE;
    }

    t_ilm_bool result = ILM_TRUE;
    int i = 0;

    for (i = 0; i < receiverCount; ++i)
    {
        int sock = (int)receiverList[i];
        result &= sendToSocket(msg, sock);
    }
    return result;
}

t_ilm_bool sendToService(t_ilm_message message)
{
    struct SocketMessage* msg = (struct SocketMessage*)message;
    if (!gState.isClient)
    {
        return ILM_FALSE;
    }

    return sendToSocket(msg, gState.socket);
}

t_ilm_message receive(t_ilm_int timeoutInMs)
{
    struct SocketMessage* queuedMessage = getFromIncomingQueue();
    if (queuedMessage)
    {
        return queuedMessage;
    }

    fd_set readFds = gState.monitoredSockets;

    int numberOfFdsReady = 0;

    if (timeoutInMs < 0)
    {
        numberOfFdsReady = select(gState.monitoredSocketMax + 1, &readFds, 0, 0, NULL);
    }
    else
    {
        struct timeval timeoutValue;
        timeoutValue.tv_sec = timeoutInMs / 1000;
        timeoutValue.tv_usec = (timeoutInMs % 1000) * 1000;
        numberOfFdsReady = select(gState.monitoredSocketMax + 1, &readFds, 0, 0, &timeoutValue);
    }

    if (-1 == numberOfFdsReady)
    {
        printf("TcpIpcModule: select() failed\n");
    }
    else if (0 < numberOfFdsReady)
    {
        int socketNumber;
        for (socketNumber = 0; socketNumber <= gState.monitoredSocketMax; ++socketNumber)
        {
            if (FD_ISSET(socketNumber, &readFds))
            {
                struct SocketMessage* msg = (struct SocketMessage*)malloc(sizeof(struct SocketMessage));
                msg->paket.type = IpcMessageTypeNone;
                msg->sender = socketNumber;
                msg->index = 0;
                addToIncomingQueue(msg);

                if (!gState.isClient)
                {
                    if (gState.socket == socketNumber)
                    {
                        // New client connected
                        msg->paket.type = IpcMessageTypeConnect;
                        acceptClientConnection();
                        continue;
                    }

                    // receive data from socket
                    receiveFromSocket(msg, socketNumber);

                    if(msg->paket.size > 0)
                    {
                        // new message from client
                        getString(msg, msg->name);
                        continue;
                    }

                    if(msg->paket.size == 0)
                    {
                        // client disconnected
                        msg->paket.type = IpcMessageTypeDisconnect;
                        close(socketNumber);
                        FD_CLR(socketNumber, &gState.monitoredSockets);
                        continue;
                    }

                    // error
                    msg->paket.type = IpcMessageTypeError;
                    const char* errorMsg = (char*)strerror(errno);
                    printf("TcpIpcModule: receive error socket %d (%s)\n", msg->sender, errorMsg);
                }
                else
                {
                    // receive LayerManager response or notification
                    receiveFromSocket(msg, socketNumber);
                    getString(msg, msg->name);
                }
            }
        }
    }

    return getFromIncomingQueue();
}

t_ilm_const_string getMessageName(t_ilm_message message)
{
    struct SocketMessage* msg = (struct SocketMessage*)message;
    return msg ? msg->name : NULL;
}

t_ilm_message_type getMessageType(t_ilm_message message)
{
    struct SocketMessage* msg = (struct SocketMessage*)message;
    return msg ? msg->paket.type : IpcMessageTypeNone;
}

t_ilm_const_string getSenderName(t_ilm_message message)
{
    struct SocketMessage* msg = (struct SocketMessage*)message;
    char name[] = "socket XXXXXXXXXXXXXXXXXX";
    sprintf(name, "socket %d", (msg ? msg->sender : -1));
    return strdup(name);
}

t_ilm_client_handle getSenderHandle(t_ilm_message message)
{
    struct SocketMessage* msg = (struct SocketMessage*)message;
    return msg ? (t_ilm_client_handle)msg->sender : (t_ilm_client_handle)0;
}


//=============================================================================
//private
//=============================================================================
t_ilm_bool acceptClientConnection()
{
    t_ilm_bool result = ILM_TRUE;
    unsigned int clientlen = sizeof(gState.clientAddrIn);

    int clientSocket = accept(gState.socket, (struct sockaddr *) &gState.clientAddrIn, &clientlen);

    if (clientSocket < 0)
    {
        printf("TcpIpcModule: accept() failed.\n");
        result = ILM_FALSE;
    }

    FD_SET(clientSocket, &gState.monitoredSockets);
    gState.monitoredSocketMax = (gState.monitoredSocketMax > clientSocket) ? gState.monitoredSocketMax : clientSocket;

    return result;
}

t_ilm_bool sendToSocket(struct SocketMessage* msg, int socketNumber)
{
    int sentBytes = 0;
    int retVal = 0;

    int headerSize = sizeof(msg->paket) - sizeof(msg->paket.data);
    msg->paket.size = msg->index + headerSize;

    int sendSize = msg->paket.size;

    do
    {
        retVal += send(socketNumber,
                       &msg->paket + sentBytes,
                       sendSize - sentBytes,
                       0);
        sentBytes += retVal;
    } while (retVal > 0 && sentBytes < sendSize);

    return (sentBytes == sendSize) ? ILM_TRUE : ILM_FALSE;
}

void receiveFromSocket(struct SocketMessage* msg, int socketNumber)
{
    int receivedBytes = 0;
    int retVal = 0;

    msg->sender = socketNumber;

    // receive header in first run (contains message size)
    msg->paket.size = sizeof(msg->paket) - sizeof(msg->paket.data);

    char* messageBuffer = (char*)&msg->paket;

    do
    {
        retVal = recv(msg->sender,
                      &messageBuffer[receivedBytes],
                      msg->paket.size - receivedBytes,
                      0);
        receivedBytes += retVal;
    } while ((retVal > 0) && (receivedBytes < msg->paket.size));

    if (0 == retVal)
    {
        // client disconnect
        msg->paket.size = 0;
    }
}

