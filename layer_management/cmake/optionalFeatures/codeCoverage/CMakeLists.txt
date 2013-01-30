############################################################################
# 
# Copyright 2010, 2011 BMW Car IT GmbH
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

# this file adds a new build-setting 'coverage',
# if all required tools are found

if(CMAKE_COMPILER_IS_GNUCC)
    message(STATUS "GCC detected, enabled 'coverage' build configuration")

    SET(GCC_COVERAGE_FLAGS "-fprofile-arcs -ftest-coverage")

    # compiler
    
    SET(CMAKE_CXX_FLAGS_COVERAGE "${CMAKE_CXX_FLAGS_DEBUG} ${GCC_COVERAGE_FLAGS}" CACHE STRING "" FORCE)
    SET(CMAKE_C_FLAGS_COVERAGE   "${CMAKE_C_FLAGS_DEBUG} ${GCC_COVERAGE_FLAGS}" CACHE STRING "" FORCE)

    # linker
    
    SET(CMAKE_EXE_LINKER_FLAGS_COVERAGE      "${CMAKE_EXE_LINKER_FLAGS_DEBUG} ${GCC_COVERAGE_FLAGS}" CACHE STRING "" FORCE)
    SET(CMAKE_MODULE_LINKER_FLAGS_COVERAGE   "${CMAKE_MODULE_LINKER_FLAGS_DEBUG} ${GCC_COVERAGE_FLAGS}" CACHE STRING "" FORCE)
    SET(CMAKE_SHARED_LINKER_FLAGS_COVERAGE   "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} ${GCC_COVERAGE_FLAGS}" CACHE STRING "" FORCE)

endif(CMAKE_COMPILER_IS_GNUCC)
