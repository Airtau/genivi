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
project(LayerManagerGraphicWaylandX11GLESv2)
project_type(CORE)

#===========================================================================
# configuration
#===========================================================================
SET(RENDERERBASE_DIR ${CMAKE_SOURCE_DIR}/LayerManagerPlugins/Renderers/Base)
SET(GRAPHIC_LIB_DIR ${CMAKE_SOURCE_DIR}/LayerManagerPlugins/Renderers/Graphic)

#===========================================================================
# component
#===========================================================================
find_package(Wayland REQUIRED)
find_package(X11 REQUIRED)

include_directories(
    ${GRAPHIC_LIB_DIR}/include
    ${RENDERERBASE_DIR}/include
    ${CMAKE_SOURCE_DIR}/config
    ${CMAKE_SOURCE_DIR}/LayerManagerBase/include
    ${CMAKE_SOURCE_DIR}/LayerManagerUtils/include
    ${X11_INCLUDE_DIR}
    ${X11_INCLUDE_DIR}
    ${WAYLAND_SERVER_INCLUDE_DIR}
)

set(SOURCES
    ${RENDERERBASE_DIR}/src/BaseRenderer.cpp
    ${GRAPHIC_LIB_DIR}/src/GraphicSystems/GLESGraphicSystem.cpp
    ${GRAPHIC_LIB_DIR}/src/WindowSystems/BaseWindowSystem.cpp
    ${GRAPHIC_LIB_DIR}/src/WindowSystems/WaylandBaseWindowSystem.cpp
    ${GRAPHIC_LIB_DIR}/src/WindowSystems/WaylandX11WindowSystem.cpp
    ${GRAPHIC_LIB_DIR}/src/WindowSystems/WaylandServerinfoProtocol.cpp
    ${GRAPHIC_LIB_DIR}/src/WindowSystems/WaylandInputDevice.cpp
    ${GRAPHIC_LIB_DIR}/src/WindowSystems/WaylandInputEvent.cpp
    ${GRAPHIC_LIB_DIR}/src/WindowSystems/WaylandX11InputEvent.cpp
    ${GRAPHIC_LIB_DIR}/src/TextureBinders/WaylandGLESTexture.cpp
)

set(LIBS
    ${LIBS}
    ${WAYLAND_SERVER_LIBRARIES}
    ${WAYLAND_CLIENT_LIBRARIES}
    ${X11_X11_LIB}
    ${WAYLAND_EGL_LIBRARIES}
    ${XKB_LIBRARIES}
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
                    ${GRAPHIC_LIB_DIR}/include/GraphicSystems/GLESGraphicSystem.h
        DESTINATION include/layermanager/graphic/GraphicSystems)

install(FILES       ${GRAPHIC_LIB_DIR}/include/TextureBinders/ITextureBinder.h
                    ${GRAPHIC_LIB_DIR}/include/TextureBinders/WaylandGLESTexture.h
        DESTINATION include/layermanager/graphic/TextureBinders)

install(FILES       ${GRAPHIC_LIB_DIR}/include/PlatformSurfaces/WaylandPlatformSurface.h
                    ${GRAPHIC_LIB_DIR}/include/PlatformSurfaces/EglWaylandPlatformSurface.h
        DESTINATION include/layermanager/graphic/PlatformSurfaces)

install(FILES       ${GRAPHIC_LIB_DIR}/include/WindowSystems/BaseWindowSystem.h
                    ${GRAPHIC_LIB_DIR}/include/WindowSystems/WaylandBaseWindowSystem.h
                    ${GRAPHIC_LIB_DIR}/include/WindowSystems/WaylandX11WindowSystem.h
        DESTINATION include/layermanager/graphic/WindowSystems)

install(FILES       ${RENDERERBASE_DIR}/include/BaseRenderer.h
                    ${RENDERERBASE_DIR}/include/RenderUtil.h
        DESTINATION include/layermanager/graphic/Base)
