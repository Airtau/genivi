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



################################################################################
#                                                                              #
#                         Layer Manager Configuration                          #
#                                                                              #
################################################################################

# Disable all GLX Examples
set (WITH_GLX_EXAMPLE OFF CACHE BOOL "" FORCE)

# Enable all EGL X11 Examples like EGLX11ApplicationExample EGLX11MockNavigation
set (WITH_EGL_EXAMPLE ON CACHE BOOL "" FORCE)

# Disable all Wayland Examples like EGLWLApplicationExample EGLWLMockNavigation
set (WITH_WL_EXAMPLE OFF CACHE BOOL "" FORCE)

# Enable EGL/GLES 2.0 X11 Renderer / Compositor Plug In
set (WITH_X11_GLES ON CACHE BOOL "" FORCE)

# Disable EGL/GLES 2.0 Wayland Renderer / Compositor Plug In
set (WITH_WAYLAND OFF CACHE BOOL "" FORCE)

# Disable EGL/GLES 2.0 Wayland Renderer / Compositor Plug In X11
set (WITH_WAYLAND_X11 OFF CACHE BOOL "" FORCE)

# Disable EGL/GLES 2.0 Wayland Renderer / Compositor Plug In FBDEV
set (WITH_WAYLAND_FBDEV OFF CACHE BOOL "" FORCE)

# Disable all OpenGL 1.1 X11 Renderer / Compositor Plug In
set (WITH_DESKTOP OFF CACHE BOOL "" FORCE)

# Disable copy of offscreen buffer into texturespace use zero copy - Driver Support needed !
set (WITH_FORCE_COPY OFF CACHE BOOL "" FORCE)

# Disable compilation of unit test
set (WITH_TESTS OFF CACHE BOOL "" FORCE)

# Enable XThreads to post signals over different threads - switching polling to push notification
set (WITH_XTRHEADS ON CACHE BOOL "" FORCE)

# Disable DLT Logging
set (WITH_DLT OFF CACHE BOOL "" FORCE)



################################################################################
#                                                                              #
#                                Cross Compilation                             #
#                                                                              #
################################################################################


#
# This present the minimal variables to be set for cross compilation.
# See http://www.cmake.org/Wiki/CMake_Cross_Compiling for more details.
#

# Set the target system type, typically Linux 
set (CMAKE_SYSTEM_NAME Linux)
# Set the C compiler to use
set (CMAKE_C_COMPILER arm-linux-gnueabi-gcc)
# Set the C++ compiler to use
set (CMAKE_CXX_COMPILER arm-linux-gnueabi-g++)
# Indicate where is located your sysroot. Can be a list of directories
set (CMAKE_FIND_ROOT_PATH  /opt/sysroot/armv7-linux-gnueabi)

# search for programs on the host
set (CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# search for headers & libraries on the target
set (CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set (CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)


