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

#ifndef _ISCENE_PROVIDER_H_
#define _ISCENE_PROVIDER_H_

class ICommandExecutor;

/**
 * Abstract Base Class for all Scene Providers to setup a inital layer Scenary
 */
class ISceneProvider
{
public:
    /**
     * constructor: any sceneprovider need a executor for commands
     * @param executor Pointer to an object to send commands to
     */
    ISceneProvider(ICommandExecutor* executor);
    virtual ~ISceneProvider() {}

    /**
     * Delegate Scene : this method will be called on LayerManager startup
     * to delegate a initial scene description of Layers on the target Platform
     */
     virtual bool delegateScene() = 0;

protected:
    ICommandExecutor* m_executor;
};

inline ISceneProvider::ISceneProvider(ICommandExecutor* executor)
: m_executor(executor)
{
}

#endif /* _ISCENE_PROVIDER_H_ */
