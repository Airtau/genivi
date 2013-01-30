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

project(LayerManagerBase)
project_type(CORE)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Xlinker -export-dynamic")

if (WITH_DLT)
    find_package(AutomotiveDlt REQUIRED)
    include_directories(${DLT_INCLUDE_DIR})
endif (WITH_DLT)

include_directories(
    include
    ${CMAKE_SOURCE_DIR}/config
    ${CMAKE_SOURCE_DIR}/LayerManagerUtils/include
    ${CMAKE_SOURCE_DIR}/LayerManagerCommands/include
)

set(SRC_FILES
    src/Configuration.cpp
    src/GraphicalObject.cpp
    src/GraphicalSurface.cpp
    src/InputManager.cpp
    src/Layermanager.cpp
    src/PluginBase.cpp
    src/PluginManager.cpp
    src/Scene.cpp
    src/SignalHandler.cpp
    src/Shader.cpp
    src/ShaderProgram.cpp
    src/ShaderProgramFactory.cpp
    src/ShaderUniform.cpp
)

add_library(${PROJECT_NAME} STATIC ${SRC_FILES})

add_dependencies(${PROJECT_NAME}
    LayerManagerUtils
)

find_package (Threads)
set(LIBS ${LIBS}
    LayerManagerUtils
    dl
    ${CMAKE_THREAD_LIBS_INIT}
    ${DLT_LIBRARY}
    ${STATICALLY_LINKED_PLUGINS}
) 

target_link_libraries(${PROJECT_NAME} ${LIBS})

file(GLOB LM_INCLUDES include/*.h)

install (FILES ${LM_INCLUDES} DESTINATION include/layermanager)


if (WITH_TESTS)

    find_package (Threads)

    enable_testing()

    add_executable(${PROJECT_NAME}_Test
        tests/SceneTest.cpp
        tests/ScreenTest.cpp
        tests/LayermanagerTest.cpp
        tests/InputManagerTest.cpp
        tests/GraphicalObjectTest.cpp
        tests/GraphicalSurfaceTest.cpp
        tests/SurfaceTest.cpp
        tests/LayerTest.cpp
        tests/ShaderTest.cpp
        tests/ShaderUniformTest.cpp
        tests/ShaderProgramTest.cpp
        tests/ShaderProgramFactoryTest.cpp
        tests/RectangleTest.cpp
    )

    target_link_libraries(${PROJECT_NAME}_Test
        ${LIBS}
        LayerManagerBase
        gtest
        gmock
        ${CMAKE_THREAD_LIBS_INIT}
    )

    add_test(${PROJECT_NAME} ${PROJECT_NAME}_Test)

endif(WITH_TESTS) 


#=============================================================
# configuration depending on system
#=============================================================
include(CheckFunctionExists)
check_function_exists(backtrace_symbols HAVE_BACKTRACE)
export_build_flag(HAVE_BACKTRACE "platform support for backtrace command")

include(CheckIncludeFiles)
check_include_files(execinfo.h HAVE_EXECINFO_H)
export_build_flag(HAVE_EXECINFO_H "platform has execinfo.h header file")

