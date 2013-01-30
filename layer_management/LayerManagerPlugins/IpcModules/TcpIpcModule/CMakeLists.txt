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

project (TcpIpcModule)
project_type(CORE)

include_directories(
    include
    ${CMAKE_SOURCE_DIR}/LayerManagerClient/ilmClient/include
    ${CMAKE_SOURCE_DIR}/LayerManagerPlugins/IpcModules/IpcModuleLoader/include
)

add_library(${PROJECT_NAME} SHARED
    src/append.c
    src/get.c
    src/initialization.c
    src/message.c
)

set(LIBS
    ${LIBS}
)

target_link_libraries(${PROJECT_NAME} ${LIBS})

install (TARGETS             ${PROJECT_NAME}
         LIBRARY DESTINATION lib/layermanager/ipcmodules
)

if (WITH_TESTS)
    add_executable(${PROJECT_NAME}_Test tests/LoopbackTest.cpp)
    target_link_libraries(${PROJECT_NAME}_Test ${LIBS} IpcModuleLoader gtest gmock pthread)
    enable_testing()
    add_test(${PROJECT_NAME} ${PROJECT_NAME}_Test)
endif(WITH_TESTS) 