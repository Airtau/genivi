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

project (DbusIpcModule)
project_type(CORE)

find_package(DBUS REQUIRED)

include_directories(
    include
    ${CMAKE_SOURCE_DIR}/LayerManagerClient/ilmClient/include
    ${CMAKE_SOURCE_DIR}/LayerManagerPlugins/IpcModules/IpcModuleLoader/include
    ${DBUS_INCLUDE_DIR}
    ${DBUS_ARCH_INCLUDE_DIR}
)

add_library(${PROJECT_NAME} SHARED
    src/bool.c
    src/double.c
    src/callbacks.c
    src/initialization.c
    src/int.c
    src/introspection.c
    src/message.c
    src/string.c
    src/uint.c
)

set(LIBS
    ${LIBS}
    ${DBUS_LIBRARY}
)

target_link_libraries(${PROJECT_NAME} ${LIBS})

install (TARGETS             ${PROJECT_NAME}
         LIBRARY DESTINATION lib/layermanager/ipcmodules)
         
install (FILES res/LayerManagerService.conf DESTINATION /etc/dbus-1/system.d)
install (FILES res/LayerManagerService.conf DESTINATION /etc/dbus-1/session.d)
         
 