/***************************************************************************
 * Copyright 2012 BMW Car IT GmbH
 * Copyright (C) 2012 DENSO CORPORATION and Robert Bosch Car Multimedia Gmbh
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

#include "Configuration.h"
#include "Log.h"
#include <getopt.h>
#include <stdlib.h>

const char* USAGE_DESCRIPTION =
    "Usage:\t LayerManagerService [options]\n"
    "options:\t\n\n"
    "\t-w: Window Width\t\n"
    "\t-h: Window Height\t\n"
    "\t-d: displayName \t\n"
    "\t-f: loglevel file \t 0 [default] \n\t\t\t\t[0=disabled, 1=error, 2=info, 3=warning, 4=debug]\n"
    "\t-c: loglevel console \t 2 [default] \n\t\t\t\t[0=disabled, 1=error, 2=info, 3=warning, 4=debug]\n"
    "\t-l: loglevel trace \t 4 [default] \n\t\t\t\t[0=disabled, 1=error, 2=info, 3=warning, 4=debug]\n"
    "\t-v: show version info\t\n"
    "\nexample: LayerManagerService -w800 -h480 -d:0\n";

Configuration::Configuration(int argc, char** argv)
: mCommandLine("")
, mDisplayWidth(DEFAULT_DISPLAY_WIDTH)
, mDisplayHeight(DEFAULT_DISPLAY_HEIGHT)
, mLogLevelConsole(DEFAULT_LOG_LEVEL_CONSOLE)
, mLogLevelFile(DEFAULT_LOG_LEVEL_FILE)
, mLogLevelTrace(DEFAULT_LOG_LEVEL_TRACE)
, mDisplayName(DEFAULT_DISPLAY_NAME)
, mPluginPath(DEFAULT_PLUGIN_PATH)
, mpBuildFlags(gBuildFlags)
, mBuildFlagCount(gBuildFlagCount)
{
    processBuildFlags();
    processEnvironmentVariables();
    processCommandLine(argc, argv);
}

std::string Configuration::getDisplayName()
{
    return mDisplayName;
}

int Configuration::getDisplayWidth()
{
    return mDisplayWidth;
}

int Configuration::getDisplayHeight()
{
    return mDisplayHeight;
}

int Configuration::getLogLevelConsole()
{
    return mLogLevelConsole;
}

int Configuration::getLogLevelFile()
{
    return mLogLevelFile;
}

int Configuration::getLogLevelTrace()
{
    return mLogLevelTrace;
}

std::string Configuration::getPluginPath()
{
    return mPluginPath;
}

void Configuration::logAllSettings()
{
    LOG_DEBUG("LayerManagerService", "Command Line: " << mCommandLine);
    LOG_INFO("LayerManagerService", "Display " << mDisplayName << " size: " << mDisplayWidth << "x" << mDisplayHeight);
    LOG_INFO("LayerManagerService", "Plugin path: " << mPluginPath);
    LOG_DEBUG("LayerManagerService", "Log level: console " << mLogLevelConsole << ", file " << mLogLevelFile << ", trace " << mLogLevelTrace);
    
    for (int i = 0; i < mBuildFlagCount; ++i)
    {
        const BuildFlag& flag = mpBuildFlags[i];
        switch (flag.type)
        {
            case DEBUG_FLAG:
                LOG_DEBUG("LayerManagerService", flag.description);
                break;
            case INFO_FLAG:
                LOG_INFO("LayerManagerService", flag.description);
                break;
        }
    }
}

void Configuration::processCommandLine(int argc, char** argv)
{
    while (optind < argc)
    {
        int option = getopt(argc, argv, "w::h::d::?::c::f::v::l::");
        switch (option)
        {
            case 'd':
                mDisplayName = optarg;
                break;
                
            case 'w':
                mDisplayWidth = atoi(optarg);
                break;
                
            case 'h':
                mDisplayHeight = atoi(optarg);
                break;
                
            case 'c':
                if (atoi(optarg) < LOG_MAX_LEVEL)
                {
                    mLogLevelConsole = (LOG_MODES)atoi(optarg);
                }
                break;
                
            case 'f':
                if (atoi(optarg) < LOG_MAX_LEVEL)
                {
                    mLogLevelFile = (LOG_MODES)atoi(optarg);
                }
                break;
                
            case 'l':
                if (atoi(optarg) < LOG_MAX_LEVEL)
                {
                    mLogLevelTrace = (LOG_MODES)atoi(optarg);
                }
                break;

            case 'v':
                LOG_INFO("LayerManagerService", "Version: " << ILM_VERSION);
                exit(-1);
                break;
                
            case '?':
            default:
                LOG_INFO("LayerManagerService", "Version: " << ILM_VERSION);
                puts(USAGE_DESCRIPTION);
                exit(-1);
        }
    }

    std::stringstream commandLine;
    for (int i = 0; i < argc; ++i)
    {
        commandLine << argv[i] << " ";
    }
    mCommandLine = commandLine.str();
}

void Configuration::processBuildFlags()
{
}

void Configuration::processEnvironmentVariables()
{
    const char* var = getenv("LM_PLUGIN_PATH");
    if (var)
    {
        mPluginPath = var;
    }
}
