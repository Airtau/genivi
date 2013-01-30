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
project_type(CORE)

include_directories(
    "include"
    "${PROJECT_SOURCE_DIR}/config"
    "${PROJECT_SOURCE_DIR}/LayerManagerUtils/include"
    "${CMAKE_SOURCE_DIR}/LayerManagerClient/ilmClient/include"
    "${CMAKE_SOURCE_DIR}/LayerManagerPlugins/IpcModules/IpcModuleLoader/include"
)

add_library(IpcModuleLoader ${LIBRARY_BUILDMODE} src/IpcModuleLoader.c)

set(LIBS
    dl
    ${LIBS}
)
target_link_libraries(IpcModuleLoader ${LIBS})

install (
    TARGETS             IpcModuleLoader
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib/layermanager/static
)
         
install (
    FILES       include/IpcModule.h
    DESTINATION include/ilm
)

if (WITH_TESTS)
#    enable_testing()
#    add_executable(IpcModuleLoader_Test tests/IlmCommandTest.cpp)
#    target_link_libraries(IpcModuleLoader_Test IpcModuleLoader ${LIBS} gtest)
#    add_test(IpcModuleLoader IpcModuleLoader_Test )
endif(WITH_TESTS) 
