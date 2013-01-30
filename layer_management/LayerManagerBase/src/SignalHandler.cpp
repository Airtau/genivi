/***************************************************************************
 * Copyright 2012 BMW Car IT GmbH
 * Copyright (C) 2012 DENSO CORPORATION and Robert Bosch Car Multimedia Gmbh
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

#include "SignalHandler.h"
#include "Log.h"
#include "config.h"

#include <signal.h>
#include <stdlib.h>   // exit
#include <pthread.h>  // mutex

#ifdef HAVE_EXECINFO_H
    #include <execinfo.h>
#endif


//===========================================================================
// global variables
//===========================================================================
static pthread_mutex_t gShutdownReceived = PTHREAD_MUTEX_INITIALIZER;

//===========================================================================
// Prototypes
//===========================================================================
void incomingSignalCallback(int sig);

//===========================================================================
// class implementation
//===========================================================================
SignalHandler::SignalHandler()
{
    LOG_DEBUG("LayerManagerService", "Enabled signal handling.");
    pthread_mutex_lock(&gShutdownReceived); // no shutdown event received
    signal(SIGBUS, incomingSignalCallback);
    signal(SIGSEGV, incomingSignalCallback);
    signal(SIGTERM, incomingSignalCallback);
    signal(SIGINT, incomingSignalCallback);
    signal(SIGABRT, incomingSignalCallback);
}

SignalHandler::~SignalHandler()
{
    LOG_DEBUG("LayerManagerService", "Disabled signal handling.");
    signal(SIGBUS, SIG_DFL);
    signal(SIGSEGV, SIG_DFL);
    signal(SIGTERM, SIG_DFL);
    signal(SIGINT, SIG_DFL);
    signal(SIGABRT, SIG_DFL);
}

void SignalHandler::waitForShutdownSignal()
{
    // this mutex is unlocked by the signal handler callback routine
    // e.g. on incoming SIGTERM or SIGABRT
    pthread_mutex_lock(&gShutdownReceived);
    pthread_mutex_unlock(&gShutdownReceived);
}

//===========================================================================
// signal handling functions
//===========================================================================
void printStackTrace()
{
#ifdef HAVE_BACKTRACE
    const int maxStackSize = 64;
    void* stack[maxStackSize];
    
    size_t count = backtrace(stack, maxStackSize);
    char **lines = backtrace_symbols(stack, count);
    
    LOG_INFO("LayerManagerService", "--------------------------------------------------");
    for (unsigned int i = 0; i < count; ++i)
    {
        LOG_INFO("LayerManagerService", "Stack-Trace [" << i << "]: " << lines[i]);
    }
    LOG_INFO("LayerManagerService", "--------------------------------------------------");
#endif

    LOG_INFO("LayerManagerService", "Exiting application.")
    exit(-1);
}

void incomingSignalCallback(int sig)
{
    switch (sig)
    {
        case SIGTERM:
            LOG_INFO("LayerManagerService", "Signal SIGTERM received. Shutting down.");
            pthread_mutex_unlock(&gShutdownReceived);
            break;
            
        case SIGINT:
            LOG_INFO("LayerManagerService", "Signal SIGINT received. Shutting down.");
            pthread_mutex_unlock(&gShutdownReceived);
            break;
            
        case SIGBUS:
            LOG_ERROR("LayerManagerService", "Signal SIGBUS received. Shutting down.");
            printStackTrace();
            pthread_mutex_unlock(&gShutdownReceived);
            break;
            
        case SIGSEGV:
            LOG_ERROR("LayerManagerService", "Signal SIGSEGV received. Shutting down.");
            printStackTrace();
            pthread_mutex_unlock(&gShutdownReceived);
            break;
            
        case SIGABRT:
            LOG_ERROR("LayerManagerService", "Signal SIGABRT received. Shutting down.");
            printStackTrace();
            pthread_mutex_unlock(&gShutdownReceived);
            break;
            
        default:
            LOG_INFO("LayerManagerService", "Signal " << sig << " received.");
            break;
    }
}
