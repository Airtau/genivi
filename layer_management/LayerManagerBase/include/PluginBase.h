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

#ifndef __PLUGINBASE_H__
#define __PLUGINBASE_H__

#include "IPlugin.h"
#include "ilm_types.h"

class ICommandExecutor;
class Configuration;

class PluginBase : public IPlugin
{
public:
    PluginBase(ICommandExecutor& executor, Configuration& config, ilmPluginApi api);
    virtual ~PluginBase();

    // from IPlugin
    virtual PluginApi pluginGetApi() const;
    virtual t_ilm_const_string pluginGetName() const;
    virtual HealthCondition pluginGetHealth();

protected:
    void pluginSetHealth(HealthCondition health);

protected:
    ICommandExecutor& mExecutor;
    Configuration& mConfiguration;

private:
    PluginApi mApi;
    HealthCondition mHealth;
};

#endif // __PLUGINBASE_H__
