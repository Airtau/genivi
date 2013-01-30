############################################################################
# 
# Copyright 2010-2012 BMW Car IT GmbH 
# Copyright (C) 2011 DENSO CORPORATION and Robert Bosch Car Multimedia Gmbh
# Copyright (C) 2012 Bayerische Motorenwerke Aktiengesellschaft
# 
# 
# Licensed under the Apache License, Version 2.0 (the "License"); 
# you may not use this file except in compliance with the License. 
# You may obtain a copy of the License at 
#
#       http://www.apache.org/licenses/LICENSE-2.0 
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
project(WaylandGLESRenderer)
project_type(PLUGIN)

find_package(Wayland REQUIRED)
find_package(EGL REQUIRED)
find_package(GLESv2 REQUIRED)
find_package(Threads REQUIRED)

SET(RENDERERBASE_DIR     ${CMAKE_SOURCE_DIR}/LayerManagerPlugins/Renderers/Base)
SET(GRAPHIC_LIB_DIR      ${CMAKE_SOURCE_DIR}/LayerManagerPlugins/Renderers/Graphic)
set(X11GLES_RENDERER_DIR ${CMAKE_SOURCE_DIR}/LayerManagerPlugins/Renderers/Platform/X11GLESRenderer)

include_directories(
    include
    ${X11GLES_RENDERER_DIR}/include
    ${RENDERERBASE_DIR}/include
    ${GRAPHIC_LIB_DIR}/include
    ${CMAKE_SOURCE_DIR}/config
    ${CMAKE_SOURCE_DIR}/LayerManagerBase/include
    ${CMAKE_SOURCE_DIR}/LayerManagerUtils/include
    ${WAYLAND_SERVER_INCLUDE_DIR}
    ${DRM_INCLUDE_DIR}
    ${EGL_INCLUDE_DIR}
    ${GLESv2_INCLUDE_DIR}
)

set(LIBS
    ${CMAKE_THREAD_LIBS_INIT}
    ${EGL_LIBRARY}
    ${GLESv2_LIBRARIES}
    LayerManagerUtils
)

set(SRC_FILES
    src/WaylandGLESRenderer.cpp
    ${X11GLES_RENDERER_DIR}/src/ShaderProgramGLES.cpp
)

#===========================================================================
# create statically linked plugin
#===========================================================================
add_library(${PROJECT_NAME} STATIC ${SRC_FILES})


#===========================================================================
# external libraries
#===========================================================================
target_link_libraries(${PROJECT_NAME} ${LIBS})

add_dependencies(${PROJECT_NAME} ${LIBS})
