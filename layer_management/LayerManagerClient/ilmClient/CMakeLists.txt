############################################################################
# 
# Copyright 2010-2012 BMW Car IT GmbH
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

project(ilmClient)
project_type(CORE)

find_package(Threads)

include_directories(
    "include"
    "${CMAKE_SOURCE_DIR}/config"
    "${CMAKE_SOURCE_DIR}/LayerManagerPlugins/IpcModules/IpcModuleLoader/include"
    "${CMAKE_SOURCE_DIR}/LayerManagerBase/include"
)

add_library(${PROJECT_NAME} SHARED
    src/generic_ilm_client.c
)

add_dependencies(${PROJECT_NAME}
    IpcModuleLoader
)

set(LIBS
    ${LIBS}
    IpcModuleLoader
    rt
    ${CMAKE_THREAD_LIBS_INIT}
)

target_link_libraries(${PROJECT_NAME} ${LIBS})

install (
    TARGETS             ${PROJECT_NAME}
    LIBRARY DESTINATION lib
)

install (
    FILES       include/ilm_client.h
                include/ilm_types.h
                include/ilm_client_platform.h
    DESTINATION include/ilm
)

if (WITH_TESTS)

    enable_testing()
    
    add_executable(${PROJECT_NAME}_Test tests/IlmCommandTest.cpp tests/IlmNotificationTest.cpp src/generic_ilm_client.c)
    target_link_libraries(${PROJECT_NAME}_Test ${PROJECT_NAME} ${LIBS} gtest)
    
    add_test(${PROJECT_NAME} ${PROJECT_NAME}_Test )

endif(WITH_TESTS) 