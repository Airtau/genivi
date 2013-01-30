############################################################################
#
# Copyright (C) 2012 Bayerische Motorenwerke Aktiengesellschaft
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


# This is an example toolchain file, please adapt for your needs
# To use that file please call
# cmake <LayerManager_Source_Dir> -DCMAKE_TOOLCHAIN_FILE=<LayerManager_Source_Dir>/toolchain.cmake

set (CMAKE_FIND_ROOT_PATH /home/user/weston)
set (CMAKE_INSTALL_PREFIX /home/user/weston)
set (FFI_INCLUDE_DIR /usr/lib/libffi-3.0.9/include)
set (XKB_INCLUDE_DIR /home/user/weston/include/xkbcommon)

################################################################################
#                                                                              #
#                         Layer Manager Configuration                          #
#                                                                              #
################################################################################
set (WITH_X11_GLES        OFF CACHE BOOL "" FORCE)
set (WITH_GLESv2_LIB      ON  CACHE BOOL "" FORCE)
set (WITH_EGL_EXAMPLE     OFF CACHE BOOL "" FORCE)
set (WITH_WL_EXAMPLE      ON  CACHE BOOL "" FORCE)
set (WITH_DESKTOP         OFF CACHE BOOL "" FORCE)
set (WITH_GLX_LIB         OFF CACHE BOOL "" FORCE)
set (WITH_GLX_EXAMPLE     OFF CACHE BOOL "" FORCE)
set (WITH_FORCE_COPY      OFF CACHE BOOL "" FORCE)
set (WITH_XTRHEADS        OFF CACHE BOOL "" FORCE)
set (WITH_CLIENTEXAMPLES  ON  CACHE BOOL "" FORCE)
set (WITH_TESTS           OFF CACHE BOOL "" FORCE)
set (WITH_DLT             OFF CACHE BOOL "" FORCE)
set (WITH_WAYLAND         ON  CACHE BOOL "" FORCE)
set (WITH_WAYLAND_X11     ON  CACHE BOOL "" FORCE)
set (WITH_WAYLAND_DRM     OFF CACHE BOOL "" FORCE)
set (WITH_WAYLAND_FBDEV   OFF CACHE BOOL "" FORCE)

set (WITH_IPC_MODULE_TCP  ON  CACHE BOOL "" FORCE)
set (WITH_IPC_MODULE_DBUS OFF CACHE BOOL "" FORCE)
