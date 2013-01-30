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

project(LayerManagerCommands)
project_type(CORE)

include_directories(
    include
    ${CMAKE_SOURCE_DIR}/config
    ${CMAKE_SOURCE_DIR}/LayerManagerBase/include
    ${CMAKE_SOURCE_DIR}/LayerManagerUtils/include
)

file(GLOB LM_SOURCES src/*.cpp)
file(GLOB LM_INCLUDES include/*.h)

add_library(${PROJECT_NAME} ${LIBRARY_BUILDMODE} ${LM_SOURCES})

set(LIBS ${LIBS} LayerManagerUtils LayerManagerBase)
target_link_libraries(${PROJECT_NAME} ${LIBS})

install (TARGETS ${PROJECT_NAME} DESTINATION lib)
install (FILES ${LM_INCLUDES} DESTINATION include/layermanager)
