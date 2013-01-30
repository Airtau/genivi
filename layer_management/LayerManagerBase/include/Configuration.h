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

#ifndef __CONFIGURATION_H__
#define __CONFIGURATION_H__

#include "config.h"
#include <string>

//============================================================================
// default configuration for LayerManagementService
//============================================================================
#define DEFAULT_DISPLAY_NAME        ":0"
#define DEFAULT_DISPLAY_WIDTH       1280
#define DEFAULT_DISPLAY_HEIGHT      480

#define DEFAULT_LOG_LEVEL_CONSOLE   2
#define DEFAULT_LOG_LEVEL_FILE      0
#define DEFAULT_LOG_LEVEL_TRACE     4

#define DEFAULT_PLUGIN_PATH         CMAKE_INSTALL_PREFIX"/lib/layermanager"

//============================================================================
// configuration class
//============================================================================
class Configuration
{
public:
    Configuration(int argc, char** argv);

    std::string getDisplayName();
    int getDisplayWidth();
    int getDisplayHeight();

    int getLogLevelConsole();
    int getLogLevelFile();
    int getLogLevelTrace();

    std::string getPluginPath();

    void logAllSettings();
    
private:
    void processCommandLine(int argc, char** argv);
    void processBuildFlags();
    void processEnvironmentVariables();

private:
    // command line arguments
    std::string mCommandLine;
    int mDisplayWidth;
    int mDisplayHeight;
    int mLogLevelConsole;
    int mLogLevelFile;
    int mLogLevelTrace;

    // environment
    std::string mDisplayName;
    std::string mPluginPath;

    // build flags
    const BuildFlag* mpBuildFlags;
    const int mBuildFlagCount;
};

#endif // __CONFIGURATION_H__
