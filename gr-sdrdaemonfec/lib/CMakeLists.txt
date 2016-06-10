#
# SDRdaemon - send I/Q samples read from a SDR device over the network via UDP
#             with FEC protection. GNUradio interface.                                               
#                                                                               
# Copyright (C) 2016 Edouard Griffiths, F4EXB                                   
#                                                                               
# This program is free software; you can redistribute it and/or modify          
# it under the terms of the GNU General Public License as published by          
# the Free Software Foundation as version 3 of the License, or                  
#                                                                               
# This program is distributed in the hope that it will be useful,               
# but WITHOUT ANY WARRANTY; without even the implied warranty of                
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                  
# GNU General Public License V3 for more details.                               
#                                                                               
# You should have received a copy of the GNU General Public License             
# along with this program. If not, see <http://www.gnu.org/licenses/>.          

#make sure our local CMake Modules path comes first
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/cmake/Modules)

########################################################################
# Setup SIMD for CM256
########################################################################
EXECUTE_PROCESS( COMMAND uname -m COMMAND tr -d '\n' OUTPUT_VARIABLE ARCHITECTURE )
message( STATUS "Architecture: ${ARCHITECTURE}" )

if(${ARCHITECTURE} MATCHES "x86_64|AMD64|x86")
    SET(USE_SIMD "SSSE3" CACHE STRING "Use SIMD SSSE3 instructions")
elseif(${ARCHITECTURE} MATCHES "armv7l")
    SET(USE_SIMD "NEON" CACHE STRING "Use SIMD NEON instructions")
endif()

if(USE_SIMD MATCHES SSSE3)
    if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANGXX)
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mssse3" )
        message(STATUS "g++ SSSE3")
        add_definitions(-DUSE_SIMD)
    endif()
elseif(USE_SIMD MATCHES NEON)
    if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANGXX)
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mfpu=neon" )
        message(STATUS "g++ NEON")
        add_definitions(-DUSE_NEON)
    endif()
endif()

if(USE_SIMD)
    message(STATUS "SIMD supported")
else(USE_SIMD)
    message(STATUS "SIMD not supported. Aborted")
    return()
endif(USE_SIMD)

########################################################################
# Specific dependencies
########################################################################
#find_package(LZ4 131 # not yet
find_package(CM256 REQUIRED)

########################################################################
# Setup library
########################################################################
include(GrPlatform) #define LIB_SUFFIX
include(GrMiscUtils)

add_library(gnuradio-sdrdaemonfec SHARED
    SDRdaemonFECBuffer.cpp
    sdrdmnfecsource_impl.cc
)

target_include_directories(gnuradio-sdrdaemonfec PUBLIC ${CM256_INCLUDE_DIR})
target_link_libraries(gnuradio-sdrdaemonfec ${Boost_LIBRARIES} ${GNURADIO_ALL_LIBRARIES} ${CM256_LIBRARIES})
set_target_properties(gnuradio-sdrdaemonfec PROPERTIES DEFINE_SYMBOL "gnuradio_SDRDAEMONFEC_EXPORTS")

########################################################################
# Install built library files
########################################################################
install(TARGETS gnuradio-sdrdaemonfec
    LIBRARY DESTINATION lib${LIB_SUFFIX} # .so/.dylib file
    ARCHIVE DESTINATION lib${LIB_SUFFIX} # .lib file
    RUNTIME DESTINATION bin              # .dll file
)

GR_LIBRARY_FOO(gnuradio-sdrdaemonfec)
