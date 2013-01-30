############################################################################
# 
# Copyright 2010-2012 BMW Car IT GmbH 
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
project(X11GLXRenderer)
project_type(PLUGIN)

find_package(X11 REQUIRED)
find_package(GLX REQUIRED)
find_package(Threads REQUIRED)

include_directories(
    include
    ../../Base/include
    ../../Graphic/include
    ${CMAKE_SOURCE_DIR}/config
    ${CMAKE_SOURCE_DIR}/LayerManagerBase/include
    ${CMAKE_SOURCE_DIR}/LayerManagerUtils/include
    ${X11_INCLUDE_DIR}
    ${GLX_INCLUDE_DIR}
)

set(LIBS
    ${CMAKE_THREAD_LIBS_INIT}
    ${X11_X11_LIB}
    ${X11_Xcomposite_LIB}
    ${X11_Xdamage_LIB}
    ${GLX_LIBRARIES}
    LayerManagerGraphicGLX
    LayerManagerUtils
)

set(SRC_FILES
    src/X11GLXRenderer.cpp
)

set(X11GLES_RENDERER_PATH ${CMAKE_SOURCE_DIR}/LayerManagerPlugins/Renderers/Platform/X11GLESRenderer)
install(FILES   ${X11GLES_RENDERER_PATH}/renderer_frag_clear.glslf
                ${X11GLES_RENDERER_PATH}/renderer_frag.glslf
                ${X11GLES_RENDERER_PATH}/renderer_frag_no_blend.glslf
                ${X11GLES_RENDERER_PATH}/renderer_frag_no_ualpha.glslf
                ${X11GLES_RENDERER_PATH}/renderer_frag_add_uchromakey.glslf
                ${X11GLES_RENDERER_PATH}/renderer_frag_no_blend_no_ualpha.glslf
                ${X11GLES_RENDERER_PATH}/renderer_frag_2surf.glslf
                ${X11GLES_RENDERER_PATH}/renderer_frag_2surf_no_blend.glslf
                ${X11GLES_RENDERER_PATH}/renderer_frag_2surf_no_ualpha.glslf
                ${X11GLES_RENDERER_PATH}/renderer_frag_2surf_no_blend_no_ualpha.glslf
                ${X11GLES_RENDERER_PATH}/renderer_frag_2surf_no_ualpha_0.glslf
                ${X11GLES_RENDERER_PATH}/renderer_frag_2surf_no_blend_no_ualpha_0.glslf
                ${X11GLES_RENDERER_PATH}/renderer_frag_2surf_no_ualpha_1.glslf
                ${X11GLES_RENDERER_PATH}/renderer_frag_2surf_no_blend_no_ualpha_1.glslf
                ${X11GLES_RENDERER_PATH}/renderer_vert.glslv
                ${X11GLES_RENDERER_PATH}/renderer_vert_2surf.glslv
    DESTINATION lib/layermanager/renderer
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






