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


include_directories (".")
include_directories ("../include")
include_directories ("${CMAKE_SOURCE_DIR}/LayerManagerService/include")
include_directories ("${CMAKE_SOURCE_DIR}/LayerManagerUtils/include")
link_directories ("${CMAKE_SOURCE_DIR}/LayerManagerCommands")

find_package(DBUS REQUIRED)
include_directories(${DBUS_INCLUDE_DIR})
include_directories(${DBUS_ARCH_INCLUDE_DIR})

find_package (Threads)
set(LIBS ${LIBS} ${DBUS_LIBRARY} ${CMAKE_THREAD_LIBS_INIT} LayerManagerUtils ${DLT_LIBRARY})

if (WITH_SYSTEMD)
    set(LIBS ${LIBS} systemd rt)
endif (WITH_SYSTEMD)

file(GLOB LM_SOURCES "${CMAKE_SOURCE_DIR}/LayerManagerService/src/Scene.cpp"
                     "${CMAKE_SOURCE_DIR}/LayerManagerService/src/Layermanager.cpp"
                     "${CMAKE_SOURCE_DIR}/LayerManagerService/src/InputManager.cpp"
                     "${CMAKE_SOURCE_DIR}/LayerManagerService/src/GraphicalObject.cpp"
                     "${CMAKE_SOURCE_DIR}/LayerManagerService/src/GraphicalSurface.cpp"
                     "${CMAKE_SOURCE_DIR}/LayerManagerService/src/HealthSystemd.cpp"
                     "${CMAKE_SOURCE_DIR}/LayerManagerService/src/PluginBase.cpp"
                     "${CMAKE_SOURCE_DIR}/LayerManagerService/src/PluginManager.cpp"
                     "${CMAKE_SOURCE_DIR}/LayerManagerService/src/Configuration.cpp"
                     "${CMAKE_SOURCE_DIR}/LayerManagerService/src/shader/*.cpp"
                     "${CMAKE_SOURCE_DIR}/LayerManagerService/src/commands/*.cpp")

file(GLOB DBUSCommunicator_SOURCES "../src/*.cpp")

set (CMAKE_CXX_FLAGS "")

add_executable(DBUSCommunicatorTest DBUSCommunicatorTest.cpp ${LM_SOURCES} ${DBUSCommunicator_SOURCES})
target_link_libraries(DBUSCommunicatorTest gmock LayerManagerCommands ${LIBS})

add_test(DBUSCommunicatorTest DBUSCommunicatorTest)
