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
#include <stdlib.h>  // getenv
#include <string.h>  // memset
#include <signal.h>
#include <unistd.h>


t_ilm_bool initServiceMode()
{
    // ignore broken pipe, if clients disconnect, handled in receive()
    signal(SIGPIPE, SIG_IGN);

    t_ilm_bool isClient = ILM_FALSE;

    t_ilm_bool result = ILM_TRUE;

    gState.isClient = isClient;

    gState.socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (gState.socket < 0)
    {
        printf("TcpIpcModule: socket()...failed\n");
        result = ILM_FALSE;
    }

    const char* portString = getenv(ENV_TCP_PORT);
    int port = SOCKET_TCP_PORT;
    if (portString)
    {
        port = atoi(portString);
    }

    gState.serverAddrIn.sin_family = AF_INET;
    gState.serverAddrIn.sin_port = htons(port);
    memset(&(gState.serverAddrIn.sin_zero), '\0', 8);

    if (gState.isClient)  // Client
    {
        const char* hostname = getenv(ENV_TCP_HOST);
        if (!hostname)
        {
            hostname = SOCKET_TCP_HOST;
        }

        struct hostent* server;
        server = gethostbyname(hostname);
        if (!server)
        {
            printf("TcpIpcModule: error: could not resolve host '%s'.\n", hostname);
            result = ILM_FALSE;
        }
        else
        {
            memcpy(&gState.serverAddrIn.sin_addr.s_addr, server->h_addr_list[0], server->h_length);
        }

        if (0 != connect(gState.socket,
                         (struct sockaddr *) &gState.serverAddrIn,
                         sizeof(gState.serverAddrIn)))
        {
            result = ILM_FALSE;
        }

        printf("TcpIpcModule: connection to %s:%d %s.\n",
                hostname, port,
                (ILM_TRUE == result) ? "established" : "failed");

        FD_SET(gState.socket, &gState.monitoredSockets);
        gState.monitoredSocketMax = gState.socket;
    }
    else  // LayerManagerService
    {
        int on = 1;
        setsockopt(gState.socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

        gState.serverAddrIn.sin_addr.s_addr = htonl(INADDR_ANY);

        if (0 > bind(gState.socket,
                    (struct sockaddr *) &gState.serverAddrIn,
                     sizeof(gState.serverAddrIn)))
        {
            printf("TcpIpcModule: bind()...failed\n");
            result = ILM_FALSE;
        }

        if (listen(gState.socket, SOCKET_MAX_PENDING_CONNECTIONS) < 0)
        {
            printf("TcpIpcModule: listen()...failed\n");
            result = ILM_FALSE;
        }

        FD_SET(gState.socket, &gState.monitoredSockets);
        gState.monitoredSocketMax = gState.socket;

        printf("TcpIpcModule: listening to TCP port: %d\n", port);
    }

    return result;
}

t_ilm_bool initClientMode()
{
    // ignore broken pipe, if clients disconnect, handled in receive()
    signal(SIGPIPE, SIG_IGN);

    t_ilm_bool isClient = ILM_TRUE;

    t_ilm_bool result = ILM_TRUE;

    gState.isClient = isClient;

    gState.socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (gState.socket < 0)
    {
        printf("TcpIpcModule: socket()...failed\n");
        result = ILM_FALSE;
    }

    const char* portString = getenv(ENV_TCP_PORT);
    int port = SOCKET_TCP_PORT;
    if (portString)
    {
        port = atoi(portString);
    }

    gState.serverAddrIn.sin_family = AF_INET;
    gState.serverAddrIn.sin_port = htons(port);
    memset(&(gState.serverAddrIn.sin_zero), '\0', 8);

    if (gState.isClient)  // Client
    {
        const char* hostname = getenv(ENV_TCP_HOST);
        if (!hostname)
        {
            hostname = SOCKET_TCP_HOST;
        }

        struct hostent* server;
        server = gethostbyname(hostname);
        if (!server)
        {
            printf("TcpIpcModule: error: could not resolve host '%s'.\n", hostname);
            result = ILM_FALSE;
        }
        else
        {
            memcpy(&gState.serverAddrIn.sin_addr.s_addr, server->h_addr_list[0], server->h_length);
        }

        if (0 != connect(gState.socket,
                         (struct sockaddr *) &gState.serverAddrIn,
                         sizeof(gState.serverAddrIn)))
        {
            result = ILM_FALSE;
        }

        printf("TcpIpcModule: connection to %s:%d %s.\n",
                hostname, port,
                (ILM_TRUE == result) ? "established" : "failed");

        FD_SET(gState.socket, &gState.monitoredSockets);
        gState.monitoredSocketMax = gState.socket;
    }
    else  // LayerManagerService
    {
        int on = 1;
        setsockopt(gState.socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

        gState.serverAddrIn.sin_addr.s_addr = htonl(INADDR_ANY);

        if (0 > bind(gState.socket,
                    (struct sockaddr *) &gState.serverAddrIn,
                     sizeof(gState.serverAddrIn)))
        {
            printf("TcpIpcModule: bind()...failed\n");
            result = ILM_FALSE;
        }

        if (listen(gState.socket, SOCKET_MAX_PENDING_CONNECTIONS) < 0)
        {
            printf("TcpIpcModule: listen()...failed\n");
            result = ILM_FALSE;
        }

        FD_SET(gState.socket, &gState.monitoredSockets);
        gState.monitoredSocketMax = gState.socket;

        printf("TcpIpcModule: listening to TCP port: %d\n", port);
    }

    return result;
}

t_ilm_bool destroy()
{
    int socketNumber;
    for (socketNumber = 0; socketNumber <= gState.monitoredSocketMax; ++socketNumber)
    {
        if (FD_ISSET(socketNumber, &gState.monitoredSockets))
        {
            printf("TcpIpcModule: Closing socket %d\n", socketNumber);
            close(socketNumber);
        }
    }

    // return to default signal handling
    signal(SIGPIPE, SIG_DFL);

    return ILM_TRUE;
}

