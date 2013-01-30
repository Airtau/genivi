############################################################################
# 
# Copyright 2010-2012 BMW Car IT GmbH 
# Copyright (C) 2012 DENSO CORPORATION and Robert Bosch Car Multimedia Gmbh
# Copyright (c) 2012, NVIDIA CORPORATION.  All rights reserved.
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

#===========================================================================
# plugin configuration
#===========================================================================
project(X11GLESRenderer)
project_type(PLUGIN)

find_package(X11 REQUIRED)
find_package(EGL REQUIRED)
find_package(GLESv2 REQUIRED)
find_package(Threads REQUIRED)

include_directories(
    include
    ../../Base/include
    ../../Graphic/include
    ${CMAKE_SOURCE_DIR}/config
    ${CMAKE_SOURCE_DIR}/LayerManagerBase/include
    ${CMAKE_SOURCE_DIR}/LayerManagerUtils/include
    ${X11_INCLUDE_DIR}
    ${EGL_INCLUDE_DIR}
    ${GLESv2_INCLUDE_DIR}
)

set(LIBS
    ${CMAKE_THREAD_LIBS_INIT}
    ${EGL_LIBRARY}
    ${GLESv2_LIBRARIES}
    ${X11_Xcomposite_LIB}
    ${X11_Xdamage_LIB}
    LayerManagerGraphicGLESv2
    LayerManagerUtils
)

set(SRC_FILES
    src/X11GLESRenderer.cpp
    src/ShaderProgramGLES.cpp
)

install(FILES
    renderer_frag_clear.glslf
    renderer_frag.glslf
    renderer_frag_no_blend.glslf
    renderer_frag_no_ualpha.glslf
    renderer_frag_add_uchromakey.glslf
    renderer_frag_no_blend_no_ualpha.glslf
    renderer_frag_2surf.glslf
    renderer_frag_2surf_no_blend.glslf
    renderer_frag_2surf_no_ualpha.glslf
    renderer_frag_2surf_no_blend_no_ualpha.glslf
    renderer_frag_2surf_no_ualpha_0.glslf
    renderer_frag_2surf_no_blend_no_ualpha_0.glslf
    renderer_frag_2surf_no_ualpha_1.glslf
    renderer_frag_2surf_no_blend_no_ualpha_1.glslf
    renderer_vert.glslv
    renderer_vert_2surf.glslv
    DESTINATION
        lib/layermanager/renderer
)

#===========================================================================
# create plugin
#===========================================================================
add_library(${PROJECT_NAME} ${LIBRARY_BUILDMODE} ${SRC_FILES})

install(TARGETS             ${PROJECT_NAME}
        LIBRARY DESTINATION lib/layermanager/renderer
        ARCHIVE DESTINATION lib/layermanager/static)

#===========================================================================
# external libraries
#===========================================================================
target_link_libraries(${PROJECT_NAME} ${LIBS})

add_dependencies(${PROJECT_NAME} ${LIBS})




























#cmake_minimum_required (VERSION 2.6)

#project(X11GLESRenderer)

#include_directories ("include")
#include_directories ("../../Base/include")
#include_directories ("../../Graphic/include")
#include_directories ("${CMAKE_SOURCE_DIR}/config")
#include_directories ("${CMAKE_SOURCE_DIR}/LayerManagerService/include")
#include_directories ("${CMAKE_SOURCE_DIR}/LayerManagerUtils/include")

#set(SRC_FILES
#    src/X11GLESRenderer.cpp
#    src/ShaderProgramGLES.cpp
#)

#if (WITH_STATIC_LIBRARIES)

#    add_library(${PROJECT_NAME} STATIC ${SRC_FILES})
#    set (STATICALLY_LINKED_PLUGINS ${STATICALLY_LINKED_PLUGINS}
#        ${PROJECT_NAME} CACHE INTERNAL "list of static plugins")

#else(WITH_STATIC_LIBRARIES)

#    add_library(${PROJECT_NAME} SHARED ${SRC_FILES})

#endif(WITH_STATIC_LIBRARIES)

#find_package(X11 REQUIRED)
#include_directories(${X11_INCLUDE_DIR})

#find_package(EGL REQUIRED)
#include_directories(${EGL_INCLUDE_DIR})

#find_package(GLESv2 REQUIRED)
#include_directories(${GLESv2_INCLUDE_DIR})

#find_package (Threads)
#set(LIBS ${LIBS} ${CMAKE_THREAD_LIBS_INIT} ${EGL_LIBRARY} ${GLESv2_LIBRARIES} ${X11_Xcomposite_LIB} ${X11_Xdamage_LIB} LayerManagerGraphicGLESv2 LayerManagerUtils LayerManagerBase)
#target_link_libraries(${PROJECT_NAME} ${LIBS})

#add_dependencies(${PROJECT_NAME} LayerManagerUtils)
#add_dependencies(${PROJECT_NAME} LayerManagerGraphicGLESv2)
#add_dependencies(${PROJECT_NAME} LayerManagerBase)

#install (TARGETS ${PROJECT_NAME}
#         LIBRARY DESTINATION lib/layermanager/renderer
#         ARCHIVE DESTINATION lib/layermanager/static
#)
#install (FILES renderer_frag_clear.glslf DESTINATION lib/layermanager/renderer)
#install (FILES renderer_frag.glslf DESTINATION lib/layermanager/renderer)
#install (FILES renderer_frag_no_blend.glslf DESTINATION lib/layermanager/renderer)
#install (FILES renderer_frag_no_ualpha.glslf DESTINATION lib/layermanager/renderer)
#install (FILES renderer_frag_add_uchromakey.glslf DESTINATION lib/layermanager/renderer)
#install (FILES renderer_frag_no_blend_no_ualpha.glslf DESTINATION lib/layermanager/renderer)
#install (FILES renderer_frag_2surf.glslf DESTINATION lib/layermanager/renderer)
#install (FILES renderer_frag_2surf_no_blend.glslf DESTINATION lib/layermanager/renderer)
#install (FILES renderer_frag_2surf_no_ualpha.glslf DESTINATION lib/layermanager/renderer)
#install (FILES renderer_frag_2surf_no_blend_no_ualpha.glslf DESTINATION lib/layermanager/renderer)
#install (FILES renderer_frag_2surf_no_ualpha_0.glslf DESTINATION lib/layermanager/renderer)
#install (FILES renderer_frag_2surf_no_blend_no_ualpha_0.glslf DESTINATION lib/layermanager/renderer)
#install (FILES renderer_frag_2surf_no_ualpha_1.glslf DESTINATION lib/layermanager/renderer)
#install (FILES renderer_frag_2surf_no_blend_no_ualpha_1.glslf DESTINATION lib/layermanager/renderer)
#install (FILES renderer_vert.glslv DESTINATION lib/layermanager/renderer)
#install (FILES renderer_vert_2surf.glslv DESTINATION lib/layermanager/renderer)
