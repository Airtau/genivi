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

#include "PluginManager.h"
#include "Configuration.h"
#include "Log.h"
#include "IPlugin.h"
#include <IScene.h>

#include <dirent.h>
#include <errno.h>
#include <dlfcn.h>
#include <stdlib.h>


StaticPluginCreateFuncList PluginManager::mStaticPluginCreateFuncList;

//===========================================================================
// register statically linked plugins (configured by build system)
//===========================================================================
STATIC_PLUGIN_REGISTRATION

//===========================================================================
// class implementation
//===========================================================================
PluginManager::PluginManager(ICommandExecutor& executor, Configuration& config)
: mExecutor(executor)
, mConfiguration(config)
{
    LOG_DEBUG("PluginManager", "loading plugins from " << mConfiguration.getPluginPath());
    
    // create static plugins
    createStaticallyLinkedPlugins();
    
#ifndef WITH_STATIC_LIBRARIES
    // create dynamic plugins
    getAllFilesInPluginPath(mConfiguration.getPluginPath());
    createDynamicallyLinkedPlugins();
#endif
    
    LOG_INFO("PluginManager", "created " << mPluginList.size() << " plugins");
}

PluginManager::~PluginManager()
{
    PluginList::iterator iter = mPluginList.begin();
    PluginList::iterator iterEnd = mPluginList.end();
    
    for (; iter != iterEnd; ++iter)
    {
        IPlugin* plugin = *iter;
        if (plugin)
        {
            LOG_INFO("PluginManager", "plugin " << plugin->pluginGetName() << " destroyed");
            delete plugin;
        }
    }
    mPluginList.clear();
}

void PluginManager::getRendererList(RendererList& list)
{
    PluginList::const_iterator iter = mPluginList.begin();
    PluginList::const_iterator iterEnd = mPluginList.end();
    
    for (; iter != iterEnd; ++iter)
    {
        IPlugin* plugin = *iter;
        ilmPluginApi api = plugin->pluginGetApi();
        if (PLUGIN_IS_RENDERER(api))
        {
            IRenderer* renderer = dynamic_cast<IRenderer*>(plugin);
            list.push_back(renderer);
        }
    }
}

void PluginManager::getHealthMonitorList(HealthMonitorList& list)
{
    PluginList::const_iterator iter = mPluginList.begin();
    PluginList::const_iterator iterEnd = mPluginList.end();
    
    for (; iter != iterEnd; ++iter)
    {
        IPlugin* plugin = *iter;
        ilmPluginApi api = plugin->pluginGetApi();
        if (PLUGIN_IS_HEALTHMONITOR(api))
        {
            IHealthMonitor* monitor = dynamic_cast<IHealthMonitor*>(plugin);
            if (monitor)
            {
                list.push_back(monitor);
            }
        }
    }
}

void PluginManager::getSceneProviderList(SceneProviderList& list)
{
    PluginList::const_iterator iter = mPluginList.begin();
    PluginList::const_iterator iterEnd = mPluginList.end();
    
    for (; iter != iterEnd; ++iter)
    {
        IPlugin* plugin = *iter;
        ilmPluginApi api = plugin->pluginGetApi();
        if (PLUGIN_IS_SCENEPROVIDER(api))
        {
            ISceneProvider* sceneprovider = dynamic_cast<ISceneProvider*>(plugin);
            list.push_back(sceneprovider);
        }
    }
}

void PluginManager::getCommunicatorList(CommunicatorList& list)
{
    PluginList::const_iterator iter = mPluginList.begin();
    PluginList::const_iterator iterEnd = mPluginList.end();
    
    for (; iter != iterEnd; ++iter)
    {
        IPlugin* plugin = *iter;
        ilmPluginApi api = plugin->pluginGetApi();
        if (PLUGIN_IS_COMMUNICATOR(api))
        {
            ICommunicator* comm = dynamic_cast<ICommunicator*>(plugin);
            list.push_back(comm);
        }
    }
}


bool PluginManager::registerStaticPluginCreateFunction(StaticPluginCreateFunc func)
{
    bool result = false;
    if (func)
    {
        mStaticPluginCreateFuncList.push_back(func);
        result = true;
    }
    return result;
}

void PluginManager::createStaticallyLinkedPlugins()
{
    StaticPluginCreateFuncList::iterator iter = mStaticPluginCreateFuncList.begin();
    StaticPluginCreateFuncList::iterator iterEnd = mStaticPluginCreateFuncList.end();
    
    for (; iter != iterEnd; ++iter)
    {
        StaticPluginCreateFunc func = *iter;
        IPlugin* plugin = (*func)(mExecutor, mConfiguration);
        LOG_INFO("PluginManager", "creating plugin " << plugin->pluginGetName() << " (static linking)");
        mPluginList.push_back(plugin);
    }
}

void PluginManager::getAllFilesInPluginPath(std::string path)
{
    DIR *directory = opendir(path.c_str());
    struct dirent *itemInDirectory = 0;
    
    while (directory && (itemInDirectory = readdir(directory)))
    {
        unsigned char entryType = itemInDirectory->d_type;
        std::string entryName = itemInDirectory->d_name;
        std::string fullPath = path + "/" + entryName;
        
        if (entryName.at(0) == '.')
        {
            continue;
        }
        
        switch (entryType)
        {
            case DT_REG:
            case DT_LNK:
            case DT_UNKNOWN:
                mFileList.push_back(fullPath);
                LOG_DEBUG("PluginManager", "considering File " << fullPath);
                break;
                
            case DT_DIR:
                getAllFilesInPluginPath(fullPath);
                break;
                
            default:
                LOG_DEBUG("PluginManager", "ignored file " << fullPath);
                break;
        }
    }
    
    closedir(directory);
}

void PluginManager::createDynamicallyLinkedPlugins()
{
    FileList::const_iterator iter = mFileList.begin();
    FileList::const_iterator iterEnd = mFileList.end();
    
    for (; iter != iterEnd; ++iter)
    {
        IPlugin* plugin = createDynamicallyLinkedPlugin(*iter);
        if (plugin)
        {
            mPluginList.push_back(plugin);
        }
    }
}

IPlugin* PluginManager::createDynamicallyLinkedPlugin(std::string path)
{
    IPlugin* returnValue = NULL;
    
    // open library
    void *libraryHandle;
    dlerror(); // Clear any existing error
    libraryHandle = dlopen(path.c_str(), RTLD_NOW | RTLD_GLOBAL);
    const char* dlopen_error = dlerror();
    if (dlopen_error)
    {
        LOG_DEBUG("PluginManager", "not a shared library: " << dlopen_error);
        return NULL;
    }
    
    // load entry point
    union
    {
        void* data;
        IPlugin* (*createFunc)(ICommandExecutor&, Configuration&);
    } convertUnion;
    
    int cutBegin = path.find_last_of('/') + 4;      // remove '*/lib' from name
    int cutEnd = path.find_first_of('.', cutBegin); // remove '.extension' from name
    
    std::string createFunctionName = "create";
    createFunctionName += path.substr(cutBegin, cutEnd - cutBegin);
    
    convertUnion.data = dlsym(libraryHandle, createFunctionName.c_str());
    
    // create plugin instance from entry point
    const char* dlsym_error = dlerror();
    if (convertUnion.data && !dlsym_error)
    {
        returnValue = convertUnion.createFunc(mExecutor, mConfiguration);
        LOG_INFO("PluginManager", "creating plugin " << returnValue->pluginGetName() << " (dynamic linking)");
    }
    else
    {
        LOG_DEBUG("PluginManager", "not a valid Plugin: " << path << ": " << dlsym_error);
        dlclose(libraryHandle);
        returnValue = NULL;
    }
    
    return returnValue;
}