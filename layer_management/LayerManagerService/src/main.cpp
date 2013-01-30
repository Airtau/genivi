/***************************************************************************
 *
 * Copyright 2010,2011 BMW Car IT GmbH
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

#include "Layermanager.h"
#include "Configuration.h"
#include "SignalHandler.h"
#include "Log.h"
#include <sys/time.h>

int main(int argc, char **argv)
{
    // monitor startup performance
    unsigned int startupTimeInMs = 0;
    timeval start, end;
    gettimeofday(&start, 0);

    // collect all configuration settings
    Configuration configuration(argc, argv);

    // setup logging
    Log::consoleLogLevel = (LOG_MODES)configuration.getLogLevelConsole();
    Log::fileLogLevel = (LOG_MODES)configuration.getLogLevelFile();
    Log::dltLogLevel = (LOG_MODES)configuration.getLogLevelTrace();

    LOG_INFO("LayerManagerService", "Starting Layermanager (version: " << ILM_VERSION << ")");
    
    // log configuration options
    configuration.logAllSettings();

    {
        SignalHandler signalHandler;
        Layermanager layermanager(configuration);

        if (layermanager.startManagement())
        {
            gettimeofday(&end, 0);
            startupTimeInMs = ((end.tv_sec - start.tv_sec) * 1000) + ((end.tv_usec - start.tv_usec) / 1000);
            LOG_INFO("LayerManagerService", "Startup complete. EnterMainloop. " << startupTimeInMs << "ms");
            signalHandler.waitForShutdownSignal();

            LOG_DEBUG("LayerManagerService", "Stopping service.");
            layermanager.stopManagement();
        }
    }

    LOG_INFO("LayerManagerService", "Shutdown complete.");
    Log::closeInstance();
    return 0;
}

