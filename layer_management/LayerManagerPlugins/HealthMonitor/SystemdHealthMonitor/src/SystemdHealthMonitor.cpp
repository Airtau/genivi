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

#include "SystemdHealthMonitor.h"
#include "Configuration.h"
#include "Log.h"
#include "ICommandExecutor.h"

#include <unistd.h>
#include <stdlib.h>

#include <systemd/sd-daemon.h>

SystemdHealthMonitor::SystemdHealthMonitor(ICommandExecutor& executor, Configuration& config)
: IHealthMonitor(executor, config)
, PluginBase(executor, config, HealthMonitor_Api_v1)
, mIntervalInMs(-1)
{
    char* envVar = getenv("WATCHDOG_USEC");
    if (envVar)
    {
        // to ms, watchdog should trigger twice the frequency
        mIntervalInMs = atoi(envVar) / 2000;
        LOG_INFO("SystemdHealthMonitor", "Watchdog interval is " << mIntervalInMs << "ms");
    }
    else
    {
        LOG_INFO("SystemdHealthMonitor", "Watchdog interval not defined, watchdog disabled");
    }
}

t_ilm_bool SystemdHealthMonitor::start()
{
    LOG_INFO("SystemdHealthMonitor", "starting");
    t_ilm_bool result = ILM_TRUE;
    if (watchdogEnabled())
    {
        result &= threadCreate();
        result &= threadInit();
        result &= threadStart();
    }
    reportStartupComplete();
    return result;
}

t_ilm_bool SystemdHealthMonitor::stop()
{
    LOG_INFO("SystemdHealthMonitor", "stopping");
    t_ilm_bool result = ILM_TRUE;
    if (watchdogEnabled())
    {
        result = threadStop();
    }
    return result;
}

t_ilm_const_string SystemdHealthMonitor::pluginGetName() const
{
    return "SystemdHealthMonitor";
}

void SystemdHealthMonitor::reportStartupComplete()
{
    LOG_INFO("SystemdHealthMonitor", "reporting startup complete");
    sd_notify(0, "READY=1");
}

void SystemdHealthMonitor::signalWatchdog()
{
    LOG_DEBUG("SystemdHealthMonitor", "Watchdog fired");
    sd_notify(0, "WATCHDOG=1");
}

bool SystemdHealthMonitor::watchdogEnabled()
{
    return (mIntervalInMs > 0);
}

t_ilm_bool SystemdHealthMonitor::threadMainLoop()
{
    if (watchdogEnabled() && PluginBase::mExecutor.getHealth())
    {
        signalWatchdog();
        usleep(mIntervalInMs * 1000);
    }
    return ILM_TRUE;
}

DECLARE_LAYERMANAGEMENT_PLUGIN(SystemdHealthMonitor)