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

#ifndef __PLUGINMANAGER_H__
#define __PLUGINMANAGER_H__

#include "PluginList.h"
#include "RendererList.h"
#include "HealthMonitorList.h"
#include "CommunicatorList.h"
#include "SceneProviderList.h"
#include "FileList.h"

class ICommandExecutor;
class Configuration;

typedef IPlugin*(*StaticPluginCreateFunc)(ICommandExecutor&, Configuration&);
typedef std::list<StaticPluginCreateFunc> StaticPluginCreateFuncList;


class PluginManager
{
public:
    PluginManager(ICommandExecutor& executor, Configuration& config);
    ~PluginManager();
    
    void getRendererList(RendererList& list);
    void getHealthMonitorList(HealthMonitorList& list);
    void getSceneProviderList(SceneProviderList& list);
    void getCommunicatorList(CommunicatorList& list);
    
    static bool registerStaticPluginCreateFunction(StaticPluginCreateFunc func);
    
private:
    void createStaticallyLinkedPlugins();
    void getAllFilesInPluginPath(std::string path);
    void createDynamicallyLinkedPlugins();
    IPlugin* createDynamicallyLinkedPlugin(std::string path);
    
private:
    ICommandExecutor& mExecutor;
    Configuration& mConfiguration;
    FileList mFileList;
    PluginList mPluginList;
    static StaticPluginCreateFuncList mStaticPluginCreateFuncList;
};

#endif // __PLUGINMANAGER_H__