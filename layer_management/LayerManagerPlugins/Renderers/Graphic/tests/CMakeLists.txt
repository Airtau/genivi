############################################################################
# 
# Copyright 2010, 2011 BMW Car IT GmbH  
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

find_package (Threads)

include_directories(
    "."
    "../include"
    "${PROJECT_SOURCE_DIR}/LayerManagerService/include"
    "${PROJECT_SOURCE_DIR}/LayerManagerUtils/include"
)

set(LIBS
    ${LIBS}
    ${CMAKE_THREAD_LIBS_INIT}
    LayerManagerUtils
)

file(GLOB LM_SOURCES
    ${PROJECT_SOURCE_DIR}/LayerManagerService/src/Scene.cpp
    ${PROJECT_SOURCE_DIR}/LayerManagerService/src/GraphicalSurface.cpp
    ${PROJECT_SOURCE_DIR}/LayerManagerService/src/GraphicalObject.cpp
    ${PROJECT_SOURCE_DIR}/LayerManagerService/src/LogicalGraphicsObject.cpp
)

enable_testing()

add_executable(ViewportTransform_Test
    ${LM_SOURCES}
    ViewportTransformTest.cpp
)

target_link_libraries(ViewportTransform_Test
    ${LIBS}
    gtest
)

add_test(ViewportTransformations ViewportTransform_Test)
