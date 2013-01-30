############################################################################
#
# Copyright 2012 BMW Car IT GmbH
#
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#               http:#www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
############################################################################

cmake_minimum_required (VERSION 2.6)

#===========================================================================
# plugin configuration
#===========================================================================
project(SystemdHealthMonitor)
project_type(PLUGIN)

include_directories(
    include
    ${CMAKE_SOURCE_DIR}/LayerManagerClient/ilmClient/include
    ${CMAKE_SOURCE_DIR}/LayerManagerBase/include
    ${CMAKE_SOURCE_DIR}/LayerManagerUtils/include
)

set(LIBS
    LayerManagerBase
    LayerManagerUtils
    systemd
    rt
)

set(SRC_FILES
    src/SystemdHealthMonitor.cpp
)

#===========================================================================
# create plugin
#===========================================================================
add_library(${PROJECT_NAME} ${LIBRARY_BUILDMODE} ${SRC_FILES})

install(TARGETS             ${PROJECT_NAME}
        LIBRARY DESTINATION lib/layermanager/health
        ARCHIVE DESTINATION lib/layermanager/static)

#===========================================================================
# external libraries
#===========================================================================
target_link_libraries(${PROJECT_NAME} ${LIBS})

add_dependencies(${PROJECT_NAME} ${LIBS})
