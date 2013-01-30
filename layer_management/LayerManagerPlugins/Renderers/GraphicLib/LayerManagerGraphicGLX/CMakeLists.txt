############################################################################
#
# Copyright 2010-2012 BMW Car IT GmbH
# Copyright (C) 2011 DENSO CORPORATION and Robert Bosch Car Multimedia Gmbh
#
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#               http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
############################################################################

cmake_minimum_required (VERSION 2.6)
project(LayerManagerGraphicGLX)
project_type(PLUGIN)

#===========================================================================
# configuration
#===========================================================================
SET(RENDERERBASE_DIR ${CMAKE_SOURCE_DIR}/LayerManagerPlugins/Renderers/Base)
SET(GRAPHIC_LIB_DIR ${CMAKE_SOURCE_DIR}/LayerManagerPlugins/Renderers/Graphic)

#===========================================================================
# component
#===========================================================================
find_package(X11 REQUIRED)
find_package(GLX REQUIRED)
find_package(Threads)

include_directories(
    ${GRAPHIC_LIB_DIR}/include
    ${RENDERERBASE_DIR}/include
    ${CMAKE_SOURCE_DIR}/config
    ${CMAKE_SOURCE_DIR}/LayerManagerBase/include
    ${CMAKE_SOURCE_DIR}/LayerManagerUtils/include
    ${X11_INCLUDE_DIR}
    ${GLX_INCLUDE_DIR}
)

set(SOURCES
    ${RENDERERBASE_DIR}/src/BaseRenderer.cpp
    ${GRAPHIC_LIB_DIR}/src/GraphicSystems/GLXGraphicSystem.cpp
    ${GRAPHIC_LIB_DIR}/src/WindowSystems/BaseWindowSystem.cpp
    ${GRAPHIC_LIB_DIR}/src/WindowSystems/X11WindowSystem.cpp
    ${GRAPHIC_LIB_DIR}/src/TextureBinders/X11TextureFromPixmap.cpp
    ${GRAPHIC_LIB_DIR}/src/TextureBinders/X11CopyGLX.cpp
    ${GRAPHIC_LIB_DIR}/src/TextureBinders/X11Copy.cpp
)

set(LIBS
    ${CMAKE_THREAD_LIBS_INIT}
    ${X11_X11_LIB}
    ${X11_Xcomposite_LIB}
    ${X11_Xdamage_LIB}
    ${GLX_LIBRARIES}
    LayerManagerUtils
    LayerManagerBase
)

add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_link_libraries(${PROJECT_NAME} ${LIBS})

add_dependencies(${PROJECT_NAME} ${LIBS})

#===========================================================================
# install
#===========================================================================
install(TARGETS     ${PROJECT_NAME}
        DESTINATION lib)

install(FILES       ${GRAPHIC_LIB_DIR}/include/GraphicSystems/BaseGraphicSystem.h
                    ${GRAPHIC_LIB_DIR}/include/GraphicSystems/GLXGraphicsystem.h
        DESTINATION include/layermanager/graphic/GraphicSystems)

install(FILES       ${GRAPHIC_LIB_DIR}/include/TextureBinders/ITextureBinder.h
                    ${GRAPHIC_LIB_DIR}/include/TextureBinders/X11Copy.h
                    ${GRAPHIC_LIB_DIR}/include/TextureBinders/X11CopyGLX.h
                    ${GRAPHIC_LIB_DIR}/include/TextureBinders/X11TextureFromPixmap.h
        DESTINATION include/layermanager/graphic/TextureBinders)

install(FILES       ${GRAPHIC_LIB_DIR}/include/PlatformSurfaces/XPlatformSurface.h
                    ${GRAPHIC_LIB_DIR}/include/PlatformSurfaces/GLXPlatformSurface.h
        DESTINATION include/layermanager/graphic/PlatformSurfaces)

install(FILES       ${GRAPHIC_LIB_DIR}/include/WindowSystems/BaseWindowSystem.h
                    ${GRAPHIC_LIB_DIR}/include/WindowSystems/X11WindowSystem.h
        DESTINATION include/layermanager/graphic/WindowSystems)

install(FILES       ${RENDERERBASE_DIR}/include/BaseRenderer.h
                    ${RENDERERBASE_DIR}/include/RenderUtil.h
        DESTINATION include/layermanager/graphic/Base)

