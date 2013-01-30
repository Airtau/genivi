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
 
#ifndef _LAYERSCENE_PROVIDER_H_
#define _LAYERSCENE_PROVIDER_H_

#include "ISceneProvider.h"
#include "PluginBase.h"

class ICommandExecutor;
class Configuration;

/**
 * scene Providers to setup a inital layer scenary on target platform
 */
class ExampleSceneProvider : public ISceneProvider, public PluginBase
{
public:
    
    ExampleSceneProvider(ICommandExecutor& executor, Configuration& config);
    virtual ~ExampleSceneProvider() {}

    // from ISceneProvider
    virtual bool delegateScene();

    //from PluginBase
    virtual t_ilm_const_string pluginGetName() const;
    
protected:
    ICommandExecutor& mExecutor;
    Configuration& mConfiguration;
};

#endif /* _LAYERSCENE_PROVIDER_H_ */
