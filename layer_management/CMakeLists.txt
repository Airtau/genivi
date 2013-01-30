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
project (LayerManager)

#===========================================================================================================
# cmake configuration
#===========================================================================================================
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/")
include(${CMAKE_MODULE_PATH}/DefaultSettings.txt)
include(${CMAKE_MODULE_PATH}/CMakeVersions.txt)
include(${CMAKE_MODULE_PATH}/CustomMacros.txt)
add_subdirectory(cmake/optionalFeatures)

#===========================================================================================================
# global build flags
#===========================================================================================================
build_flag (WITH_FORCE_COPY       "Force Software Copy of Pixmaps (compatibility for VMs)" OFF)
build_flag (WITH_DLT              "Build with DLT logging support"                         OFF)
build_flag (WITH_STATIC_LIBRARIES "Link all plugins and libraries statically"              OFF)

#===========================================================================================================
build_flag (WITH_TESTS "Build unit test binaries for all enabled components" OFF)
#===========================================================================================================
if (WITH_TESTS)
    # enable ctest if building with tests, run 'make test' after build to run all tests
    # be aware that some tests require a running layermanager
    enable_testing()
    add_subdirectory_once (3rdParty/gtest)
    add_subdirectory_once (3rdParty/gmock)
endif(WITH_TESTS)

#===========================================================================================================
build_flag (WITH_EGL_EXAMPLE "Build examples for GLES/X11 based platforms" ON)
#===========================================================================================================
if (WITH_EGL_EXAMPLE)
    add_subdirectory_once (LayerManagerUtils)
    add_subdirectory_once (LayerManagerClient/ilmClient)
    add_subdirectory_once (LayerManagerExamples/EGLX11ApplicationExample)
    add_subdirectory_once (LayerManagerExamples/EGLX11MockNavigation)
endif(WITH_EGL_EXAMPLE)

#===========================================================================================================
build_flag (WITH_GLX_EXAMPLE "Build examples for OpenGL/X11 based platforms" OFF)
#===========================================================================================================
if (WITH_GLX_EXAMPLE)
    add_subdirectory_once (LayerManagerUtils)
    add_subdirectory_once (LayerManagerClient/ilmClient)
    add_subdirectory_once (LayerManagerExamples/GLXApplicationExample)
endif(WITH_GLX_EXAMPLE)

#===========================================================================================================
build_flag (WITH_CLIENTEXAMPLES "Build examples for client library usage" ON)
#===========================================================================================================
if (WITH_CLIENTEXAMPLES)
    add_subdirectory_once (LayerManagerUtils)
    add_subdirectory_once (LayerManagerClient/ilmClient)
    add_subdirectory_once (LayerManagerExamples/LayerManagerClientExample)
endif(WITH_CLIENTEXAMPLES)

#===========================================================================================================
build_flag (WITH_WL_EXAMPLE "Build examples for GLES/Wayland based platforms" OFF)
#===========================================================================================================
if (WITH_WL_EXAMPLE)
    add_subdirectory_once (LayerManagerUtils)
    add_subdirectory_once (LayerManagerClient/ilmClient)
    add_subdirectory_once (LayerManagerExamples/EGLWLApplicationExample)
    add_subdirectory_once (LayerManagerExamples/EGLWLMockNavigation)
    add_subdirectory_once (LayerManagerExamples/WLChromakeyExample)
    add_subdirectory_once (LayerManagerExamples/EGLWLInputEventExample)
endif(WITH_WL_EXAMPLE)

#===========================================================================================================
build_flag (WITH_CONTROL_BIN "Build LayerManagerControl binary" ON)
#===========================================================================================================
if (WITH_CONTROL_BIN)
    add_subdirectory_once (LayerManagerUtils)
    add_subdirectory_once (LayerManagerClient/ilmClient)
    add_subdirectory_once (LayerManagerControl)
endif(WITH_CONTROL_BIN)

#===========================================================================================================
build_flag (WITH_EXAMPLE_SCENE_PROVIDER "Build scene provider plugin to run example applications" OFF)
#===========================================================================================================
if (WITH_EXAMPLE_SCENE_PROVIDER)
    add_subdirectory_once (LayerManagerCommands)
    add_subdirectory_once (LayerManagerBase)
    add_subdirectory_once (LayerManagerPlugins/SceneProvider/ExampleSceneProvider)
endif(WITH_EXAMPLE_SCENE_PROVIDER)

#===========================================================================================================
build_flag (WITH_SYSTEMD_HEALTH_MONITOR "Build plugin for systemd based health monitoring" OFF)
#===========================================================================================================
if (WITH_SYSTEMD_HEALTH_MONITOR)
    add_subdirectory_once (3rdParty/systemd)
    add_subdirectory_once (LayerManagerUtils)
    add_subdirectory_once (LayerManagerBase)
    add_subdirectory_once (LayerManagerPlugins/HealthMonitor/SystemdHealthMonitor)
endif(WITH_SYSTEMD_HEALTH_MONITOR)

#===========================================================================================================
build_flag (WITH_GENERIC_COMMUNICATOR "Build Generic Communicator Plugin based on IpcModules" ON)
#===========================================================================================================
if (WITH_GENERIC_COMMUNICATOR)
    add_subdirectory_once (LayerManagerPlugins/IpcModules)
    add_subdirectory_once (LayerManagerCommands)
    add_subdirectory_once (LayerManagerPlugins/Communicators/GenericCommunicator)
endif(WITH_GENERIC_COMMUNICATOR)

#===========================================================================================================
build_flag (WITH_DESKTOP "Build renderer plugin for OpenGL/X11 based platforms" OFF)
#===========================================================================================================
if (WITH_DESKTOP)
    add_subdirectory_once (LayerManagerPlugins/Renderers/GraphicLib/LayerManagerGraphicGLX)
    add_subdirectory_once (LayerManagerPlugins/Renderers/Platform/GLXRenderer)
endif(WITH_DESKTOP)

#===========================================================================================================
build_flag (WITH_GLX_LIB "Build development library for OpenGL/X11 based renderers" OFF)
#===========================================================================================================
if (WITH_GLX_LIB)
    add_subdirectory_once (LayerManagerPlugins/Renderers/GraphicLib/LayerManagerGraphicGLX)
endif(WITH_GLX_LIB)

#===========================================================================================================
build_flag (WITH_WAYLAND_X11 "Build renderer plugin for GLES/Wayland with X11 backend" OFF)
#===========================================================================================================
if (WITH_WAYLAND_X11)
    add_subdirectory_once (LayerManagerUtils)
    add_subdirectory_once (LayerManagerBase)
    add_subdirectory_once (LayerManagerPlugins/Renderers/GraphicLib/LayerManagerGraphicWaylandX11GLESv2)
    add_subdirectory_once (LayerManagerPlugins/Renderers/Platform/WaylandGLESRenderer)
    add_subdirectory_once (LayerManagerPlugins/Renderers/Platform/WaylandX11GLESRenderer)
endif(WITH_WAYLAND_X11)

#===========================================================================================================
build_flag (WITH_WAYLAND_X11_LIB "Build development library for GLES/Wayland X11 based renderers" OFF)
#===========================================================================================================
if (WITH_WAYLAND_X11_LIB)
    add_subdirectory_once (LayerManagerPlugins/Renderers/GraphicLib/LayerManagerGraphicWaylandX11GLESv2)
endif(WITH_WAYLAND_X11_LIB)

#===========================================================================================================
build_flag (WITH_WAYLAND_DRM "Build renderer plugin for GLES/Wayland with DRM backend" OFF)
#===========================================================================================================
if (WITH_WAYLAND_DRM)
    add_subdirectory_once (LayerManagerUtils)
    add_subdirectory_once (LayerManagerBase)
    add_subdirectory_once (LayerManagerPlugins/Renderers/GraphicLib/LayerManagerGraphicWaylandDrmGLESv2)
    add_subdirectory_once (LayerManagerPlugins/Renderers/Platform/WaylandGLESRenderer)
    add_subdirectory_once (LayerManagerPlugins/Renderers/Platform/WaylandDrmGLESRenderer)
endif(WITH_WAYLAND_DRM)

#===========================================================================================================
build_flag (WITH_WAYLAND_DRM_LIB "Build development library for GLES/Wayland DRM based renderers" OFF)
#===========================================================================================================
if (WITH_WAYLAND_DRM_LIB)
    add_subdirectory_once (LayerManagerPlugins/Renderers/GraphicLib/LayerManagerGraphicWaylandDrmGLESv2)
endif(WITH_WAYLAND_DRM_LIB)

#===========================================================================================================
build_flag (WITH_WAYLAND_FBDEV "Build renderer plugin for GLES/Wayland with FBDEV backend" OFF)
#===========================================================================================================
if (WITH_WAYLAND_FBDEV)
    add_subdirectory_once (LayerManagerUtils)
    add_subdirectory_once (LayerManagerBase)
    add_subdirectory_once (LayerManagerPlugins/Renderers/GraphicLib/LayerManagerGraphicWaylandFbdevGLESv2)
    add_subdirectory_once (LayerManagerPlugins/Renderers/Platform/WaylandGLESRenderer)
    add_subdirectory_once (LayerManagerPlugins/Renderers/Platform/WaylandFbdevGLESRenderer)
endif(WITH_WAYLAND_FBDEV)

#===========================================================================================================
build_flag (WITH_WAYLAND_FBDEV_LIB "Build development library for GLES/Wayland FBDEV based renderers" OFF)
#===========================================================================================================
if (WITH_WAYLAND_FBDEV_LIB)
    add_subdirectory_once (LayerManagerPlugins/Renderers/GraphicLib/LayerManagerGraphicWaylandFbdevGLESv2)
endif(WITH_WAYLAND_FBDEV_LIB)

#===========================================================================================================
build_flag (WITH_X11_GLES "Build renderer plugin for GLES/X11 based platforms" ON)
#===========================================================================================================
if (WITH_X11_GLES)
    add_subdirectory_once (LayerManagerPlugins/Renderers/GraphicLib/LayerManagerGraphicGLESv2)
    add_subdirectory_once (LayerManagerPlugins/Renderers/Platform/X11GLESRenderer)
endif(WITH_X11_GLES)

#===========================================================================================================
build_flag (WITH_GLESv2_LIB "Build development library for GLES/X11 based renderers" OFF)
#===========================================================================================================
if (WITH_GLESv2_LIB)
    add_subdirectory_once (LayerManagerPlugins/Renderers/GraphicLib/LayerManagerGraphicGLESv2)
endif(WITH_GLESv2_LIB)

#===========================================================================================================
build_flag (WITH_TEXT_RENDERER "Build renderer plugin with pure logging (no rendering)" OFF)
#===========================================================================================================
if (WITH_TEXT_RENDERER)
    add_subdirectory_once (LayerManagerPlugins/Renderers/Platform/TextRenderer)
endif(WITH_TEXT_RENDERER)

#===========================================================================================================
build_flag (WITH_SERVICE_BIN "Build LayerManagerService binary" ON)
#===========================================================================================================
if (WITH_SERVICE_BIN)
    add_subdirectory_once (LayerManagerUtils)
    add_subdirectory_once (LayerManagerBase)
    add_subdirectory_once (LayerManagerService)
endif (WITH_SERVICE_BIN)

#===========================================================================================================
build_flag (WITH_CLIENT_LIB "Build LayerManagement client library" ON)
#===========================================================================================================
if (WITH_CLIENT_LIB)
    add_subdirectory_once (LayerManagerPlugins/IpcModules)
    add_subdirectory_once (LayerManagerClient/ilmClient)
endif (WITH_CLIENT_LIB)

#===========================================================================================================
# store used build configuration
#===========================================================================================================
add_subdirectory_once(config)
