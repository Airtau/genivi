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

#ifndef __HEALTHSYSTEMD_H__
#define __HEALTHSYSTEMD_H__

#include "IHealthMonitor.h"
#include "ThreadBase.h"
#include "PluginBase.h"

class SystemdHealthMonitor : public IHealthMonitor, protected ThreadBase, public PluginBase
{
public:
    SystemdHealthMonitor(ICommandExecutor& executor, Configuration& config);

    // from IHealthMonitor
    virtual t_ilm_bool start();
    virtual t_ilm_bool stop();

    //from PluginBase
    virtual t_ilm_const_string pluginGetName() const;

private:
    void reportStartupComplete();
    void reportProcessId();
    int getWatchdogIntervalInMs();
    void signalWatchdog();
    bool watchdogEnabled();

    // from ThreadBase
    virtual t_ilm_bool threadMainLoop();
    
private:
    int mIntervalInMs;
};

#endif // __HEALTHSYSTEMD_H__