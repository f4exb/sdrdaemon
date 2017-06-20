# Copyright 2011,2012,2016 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
#
# GNU Radio is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# GNU Radio is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Radio; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.

#make sure our local CMake Modules path comes first
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/cmake/Modules)

########################################################################
# Setup SIMD for CM256
########################################################################
EXECUTE_PROCESS( COMMAND uname -m COMMAND tr -d '\n' OUTPUT_VARIABLE ARCHITECTURE )
message( STATUS "Architecture: ${ARCHITECTURE}" )

if (${ARCHITECTURE} MATCHES "x86_64|AMD64|x86")
    EXECUTE_PROCESS( COMMAND grep flags /proc/cpuinfo OUTPUT_VARIABLE CPU_FLAGS )
    if (${CPU_FLAGS} MATCHES "avx2")
        set(HAS_AVX2 ON CACHE BOOL "Architecture has AVX2 SIMD enabled")
        if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANGXX)
            set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2" )
            message(STATUS "Use g++ AVX2 SIMD instructions")
            add_definitions(-DUSE_AVX2)
        endif()
    else()
        set(HAS_AVX2 OFF CACHE BOOL "Architecture does not have AVX2 SIMD enabled")
    endif()
    if (${CPU_FLAGS} MATCHES "sse4_1")
        set(HAS_SSE4_1 ON CACHE BOOL "Architecture has SSE 4.1 SIMD enabled")
        if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANGXX)
            set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -msse4.1" )
            message(STATUS "Use g++ SSE 4.1 SIMD instructions")
            add_definitions(-DUSE_SSE4_1)
        endif()
    else()
        set(HAS_SSE4_1 OFF CACHE BOOL "Architecture does not have SSE 4.1 SIMD enabled")
    endif()
    if (${CPU_FLAGS} MATCHES "ssse3")
        set(HAS_SSSE3 ON CACHE BOOL "Architecture has SSSE3 SIMD enabled")
        if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANGXX)
            set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mssse3" )
            message(STATUS "Use g++ SSSE3 SIMD instructions")
            add_definitions(-DUSE_SSSE3)
        endif()
    else()
        set(HAS_SSSE3 OFF CACHE BOOL "Architecture does not have SSSE3 SIMD enabled")
    endif()
elseif (${ARCHITECTURE} MATCHES "armv7l")
    EXECUTE_PROCESS( COMMAND grep Features /proc/cpuinfo OUTPUT_VARIABLE CPU_FLAGS )
    if (${CPU_FLAGS} MATCHES "neon")
        set(HAS_NEON ON CACHE BOOL "Architecture has NEON SIMD enabled")
        if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANGXX)
            set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mfpu=neon" )
            message(STATUS "Use g++ NEON SIMD instructions")
            add_definitions(-DUSE_NEON)
        endif()
    else()
        set(HAS_NEON OFF CACHE BOOL "Architecture does not have NEON SIMD enabled")
    endif()
endif()

########################################################################
# Specific dependencies
########################################################################
find_package(CM256 REQUIRED)

if (HAS_SSSE3 OR HAS_NEON)
    message(STATUS "gr-sdaemon with SIMD instructions enabled")
endif()

########################################################################
# Setup library
########################################################################
include(GrPlatform) #define LIB_SUFFIX

include_directories(${Boost_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})

list(APPEND sdrdaemon_sources
    SDRdaemonFECBuffer.cpp
    sdrdaemonsource_impl.cc
    sdrdaemonsink_impl.cc
)

set(sdrdaemon_sources "${sdrdaemon_sources}" PARENT_SCOPE)
if(NOT sdrdaemon_sources)
	MESSAGE(STATUS "No C++ sources... skipping lib/")
	return()
endif(NOT sdrdaemon_sources)

add_library(gnuradio-sdrdaemon SHARED ${sdrdaemon_sources})
target_include_directories(gnuradio-sdrdaemon PUBLIC ${CM256_INCLUDE_DIR})
target_link_libraries(gnuradio-sdrdaemon ${Boost_LIBRARIES} ${GNURADIO_ALL_LIBRARIES})
set_target_properties(gnuradio-sdrdaemon PROPERTIES DEFINE_SYMBOL "gnuradio_sdrdaemon_EXPORTS")

if(APPLE)
    set_target_properties(gnuradio-sdrdaemon PROPERTIES
        INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib"
    )
endif(APPLE)

########################################################################
# Install built library files
########################################################################
include(GrMiscUtils)
GR_LIBRARY_FOO(gnuradio-sdrdaemon RUNTIME_COMPONENT "sdrdaemon_runtime" DEVEL_COMPONENT "sdrdaemon_devel")

########################################################################
# Build and register unit test
########################################################################
include(GrTest)

include_directories(${CPPUNIT_INCLUDE_DIRS})

list(APPEND test_sdrdaemon_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/test_sdrdaemon.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/qa_sdrdaemon.cc
)

add_executable(test-sdrdaemon ${test_sdrdaemon_sources})

target_link_libraries(
  test-sdrdaemon
  ${GNURADIO_RUNTIME_LIBRARIES}
  ${Boost_LIBRARIES}
  ${CPPUNIT_LIBRARIES}
  gnuradio-sdrdaemon
)

GR_ADD_TEST(test_sdrdaemon test-sdrdaemon)

########################################################################
# Print summary
########################################################################
message(STATUS "Using install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Building for version: ${VERSION} / ${LIBVER}")

