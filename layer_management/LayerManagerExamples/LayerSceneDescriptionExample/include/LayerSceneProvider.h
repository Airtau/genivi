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
 
#ifndef _LAYERSCENE_PROVIDER_H_
#define _LAYERSCENE_PROVIDER_H_

#include "ISceneProvider.h"

class ICommandExecutor;

/**
 * scene Providers to setup a inital layer scenary on target platform
 */
class ExampleSceneProvider : public ISceneProvider
{
public:
    
    ExampleSceneProvider(ICommandExecutor* executor);
    virtual ~ExampleSceneProvider() {}
    /**
     * Delegate Scene : this method will be called on LayerManager startup
     * to delegate a initial scene description of Layers on the target Platform
     */
    virtual bool delegateScene();
};


#endif /* _LAYERSCENE_PROVIDER_H_ */
