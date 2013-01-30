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

#ifndef __IHEALTH_H__
#define __IHEALTH_H__

#include "ilm_types.h"

class ICommandExecutor;
class Configuration;

class IHealthMonitor
{
public:
    IHealthMonitor(ICommandExecutor& executor, Configuration& config);
    virtual ~IHealthMonitor();

    virtual t_ilm_bool start() = 0;
    virtual t_ilm_bool stop() = 0;

protected:
    ICommandExecutor& mExecutor;
    Configuration& mConfiguration;
};

inline
IHealthMonitor::IHealthMonitor(ICommandExecutor& executor, Configuration& config)
: mExecutor(executor)
, mConfiguration(config)
{
}

inline
IHealthMonitor::~IHealthMonitor()
{
}

#endif // __IHEALTH_H__
