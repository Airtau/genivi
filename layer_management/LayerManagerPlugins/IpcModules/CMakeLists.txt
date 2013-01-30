############################################################################
# 
# Copyright 2012 BMW Car IT GmbH
# 
# 
# Licensed under the Apache License, Version 2.0 (the "License"); 
# you may not use this file except in compliance with the License. 
# You may obtain a copy of the License at 
#
#		http://www.apache.org/licenses/LICENSE-2.0 
#
# Unless required by applicable law or agreed to in writing, software 
# distributed under the License is distributed on an "AS IS" BASIS, 
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
# See the License for the specific language governing permissions and 
# limitations under the License.
#
############################################################################

cmake_minimum_required (VERSION 2.6)

set (BUILD_LOADER OFF)

#==============================================================================
# TCP Socket IPC MODULE
#==============================================================================
option (WITH_IPC_MODULE_TCP "Build with TCP Ipc Module" OFF)

if (WITH_IPC_MODULE_TCP)
    add_subdirectory(TcpIpcModule)
    set (BUILD_LOADER ON)
endif (WITH_IPC_MODULE_TCP)

#==============================================================================
# DBUS IPC MODULE
#==============================================================================
option (WITH_IPC_MODULE_DBUS "Build with D-Bus Ipc Module" ON)

if (WITH_IPC_MODULE_DBUS)
    add_subdirectory(DbusIpcModule)
    set (BUILD_LOADER ON)
endif (WITH_IPC_MODULE_DBUS)

#==============================================================================
# IPC MODULE LOADER
#==============================================================================
if (BUILD_LOADER)
    add_subdirectory(IpcModuleLoader)
endif (BUILD_LOADER)
