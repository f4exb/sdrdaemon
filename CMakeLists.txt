cmake_minimum_required (VERSION 3.0.2)

# use, i.e. don't skip the full RPATH for the build tree
set(CMAKE_SKIP_BUILD_RPATH  FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)

project (sdrdaemon)

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
    message(STATUS "SIMD defined")
else(USE_SIMD)
    message(STATUS "SIMD not defined")
endif(USE_SIMD)

find_package(Threads)
find_package(PkgConfig)
find_package(Boost 1.47)
find_package(LZ4 131)
find_package(CM256cc)

# Find Airspy library.
pkg_check_modules(PKG_AIRSPY libairspy)
find_path(LIBAIRSPY_INCLUDE_DIR airspy.h
          HINT ${PKG_LIBAIRSPY_INCLUDE_DIR})
find_library(LIBAIRSPY_LIBRARIES libairspy.so
          HINT ${PKG_LIBAIRSPY_LIBRARIES_DIRS})
if(LIBAIRSPY_INCLUDE_DIR AND LIBAIRSPY_LIBRARIES)
    set(LIBAIRSPY_FOUND TRUE CACHE INTERNAL "libairspy found")
    message(STATUS "Found libairspy: ${LIBAIRSPY_INCLUDE_DIR}, ${LIBAIRSPY_LIBRARIES}")
else(LIBAIRSPY_INCLUDE_DIR AND LIBAIRSPY_LIBRARIES)
    set(LIBAIRSPY_FOUND FALSE CACHE INTERNAL "libairspy found")
    message(STATUS "libairspy not found")
endif(LIBAIRSPY_INCLUDE_DIR AND LIBAIRSPY_LIBRARIES)

# Find BladeRF library.
pkg_check_modules(PKG_BLADERF libbladerf)
find_path(LIBBLADERF_INCLUDE_DIR libbladeRF.h
          HINT ${PKG_LIBBLADERF_INCLUDE_DIR})
find_library(LIBBLADERF_LIBRARIES libbladeRF.so
          HINT ${PKG_LIBBLADERF_LIBRARIES_DIRS})
if(LIBBLADERF_INCLUDE_DIR AND LIBBLADERF_LIBRARIES)
    set(LIBBLADERF_FOUND TRUE CACHE INTERNAL "libbladerf found")
    message(STATUS "Found libbladerf: ${LIBBLADERF_INCLUDE_DIR}, ${LIBBLADERF_LIBRARIES}")
else(LIBBLADERF_INCLUDE_DIR AND LIBBLADERF_LIBRARIES)
    set(LIBBLADERF_FOUND FALSE CACHE INTERNAL "libbladerf found")
    message(STATUS "libbladerf not found")
endif(LIBBLADERF_INCLUDE_DIR AND LIBBLADERF_LIBRARIES)

# Find HackRF library.
pkg_check_modules(PKG_HACKRF libhackrf)
find_path(LIBHACKRF_INCLUDE_DIR hackrf.h
          HINT ${PKG_LIBHACKRF_INCLUDE_DIRS})
find_library(LIBHACKRF_LIBRARIES libhackrf.so
          HINT ${PKG_LIBHACKRF_LIBRARIES_DIRS})
if(LIBHACKRF_INCLUDE_DIR AND LIBHACKRF_LIBRARIES)
    set(LIBHACKRF_FOUND TRUE CACHE INTERNAL "libhackrf found")
    message(STATUS "Found libhackrf: ${LIBHACKRF_INCLUDE_DIR}, ${LIBHACKRF_LIBRARIES}")
else(LIBHACKRF_INCLUDE_DIR AND LIBHACKRF_LIBRARIES)
    set(LIBHACKRF_FOUND FALSE CACHE INTERNAL "libhackrf found")
    message(STATUS "libhackrf not found")
endif(LIBHACKRF_INCLUDE_DIR AND LIBHACKRF_LIBRARIES)

# Find RTL-SDR library.
pkg_check_modules(PKG_RTLSDR librtlsdr)
find_path(LIBRTLSDR_INCLUDE_DIR rtl-sdr.h
          HINT ${PKG_LIBRTLSDR_INCLUDE_DIRS})
find_library(LIBRTLSDR_LIBRARIES librtlsdr.so
          HINT ${PKG_LIBRTLSDR_LIBRARIES_DIRS})
if(LIBRTLSDR_INCLUDE_DIR AND LIBRTLSDR_LIBRARIES)
    set(LIBRTLSDR_FOUND TRUE CACHE INTERNAL "librtlsdr found")
    message(STATUS "Found librtlsdr: ${LIBRTLSDR_INCLUDE_DIR}, ${LIBRTLSDR_LIBRARIES}")
else(LIBRTLSDR_INCLUDE_DIR AND LIBRTLSDR_LIBRARIES)
    set(LIBRTLSDR_FOUND FALSE CACHE INTERNAL "librtlsdr found")
    message(STATUS "librtlsdr not found")
endif(LIBRTLSDR_INCLUDE_DIR AND LIBRTLSDR_LIBRARIES)

# Find libusb
pkg_check_modules(PKG_LIBUSB libusb-1.0)
find_path(LIBUSB_INCLUDE_DIR libusb.h
          HINT ${PKG_LIBUSB_INCLUDE_DIRS}
          PATH_SUFFIXES libusb-1.0)
find_library(LIBUSB_LIBRARY usb-1.0
          HINT ${PKG_LIBUSB_LIBRARY_DIRS})
if(LIBUSB_INCLUDE_DIR AND LIBUSB_LIBRARY)
    set(LIBUSB_FOUND TRUE CACHE INTERNAL "libusb found")
    message(STATUS "Found libusb: ${LIBUSB_INCLUDE_DIR}, ${LIBUSB_LIBRARY}")
else(LIBUSB_INCLUDE_DIR AND LIBUSB_LIBRARY)
    set(LIBRTLSDR_FOUND FALSE CACHE INTERNAL "libusb found")
    message(STATUS "libusb not found")
endif(LIBUSB_INCLUDE_DIR AND LIBUSB_LIBRARY)

# Find nanomsg library.
pkg_check_modules(PKG_LIBNANOMSG libnanomsg)
find_path(LIBNANOMSG_INCLUDE_DIR nanomsg/nn.h
          HINT ${PKG_LIBNANOMSG_INCLUDE_DIRS})
find_library(LIBNANOMSG_LIBRARIES libnanomsg.so
          HINT ${PKG_LIBNANOMSG_LIBRARIES_DIRS})
if(LIBNANOMSG_INCLUDE_DIR AND LIBNANOMSG_LIBRARIES)
    set(LIBNANOMSG_FOUND TRUE CACHE INTERNAL "libnanomsg found")
    message(STATUS "Found libnanomsg: ${LIBNANOMSG_INCLUDE_DIR}, ${LIBNANOMSG_LIBRARIES}")
else(LIBNANOMSG_INCLUDE_DIR AND LIBNANOMSG_LIBRARIES)
    set(LIBNANOMSG_FOUND FALSE CACHE INTERNAL "libnanomsg found")
    message(FATAL_ERROR "libnanomsg is not found and is required. CMake will exit")
endif(LIBNANOMSG_INCLUDE_DIR AND LIBNANOMSG_LIBRARIES)

set(LIBAIRSPY_INCLUDE_DIRS ${LIBAIRSPY_INCLUDE_DIR} ${LIBUSB_INCLUDE_DIR} ${LIBNANOMSG_INCLUDE_DIR})
set(LIBAIRSPY_LIBRARIES    ${LIBAIRSPY_LIBRARIES} ${LIBUSB_LIBRARY} ${LIBNANOMSG_LIBRARIES})

set(LIBBLADERF_INCLUDE_DIRS ${LIBBLADERF_INCLUDE_DIR} ${LIBUSB_INCLUDE_DIR} ${LIBNANOMSG_INCLUDE_DIR})
set(LIBBLADERF_LIBRARIES    ${LIBBLADERF_LIBRARIES} ${LIBUSB_LIBRARY} ${LIBNANOMSG_LIBRARIES})

set(LIBHACKRF_INCLUDE_DIRS ${LIBHACKRF_INCLUDE_DIR} ${LIBUSB_INCLUDE_DIR} ${LIBNANOMSG_INCLUDE_DIR})
set(LIBHACKRF_LIBRARIES    ${LIBHACKRF_LIBRARIES} ${LIBUSB_LIBRARY} ${LIBNANOMSG_LIBRARIES})

set(LIBRTLSDR_INCLUDE_DIRS ${LIBRTLSDR_INCLUDE_DIR} ${LIBUSB_INCLUDE_DIR} ${LIBNANOMSG_INCLUDE_DIR})
set(LIBRTLSDR_LIBRARIES    ${LIBRTLSDR_LIBRARIES} ${LIBUSB_LIBRARY} ${LIBNANOMSG_LIBRARIES})

set(LIBTEST_INCLUDE_DIRS ${LIBNANOMSG_INCLUDE_DIR})
set(LIBTEST_LIBRARIES    ${LIBNANOMSG_LIBRARIES})

# Compiler flags.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11 -O2 -ffast-math -ftree-vectorize ${EXTRA_FLAGS}")

set(sdmnbase_SOURCES
    sdmnbase/CRC64.cpp
    sdmnbase/Decimators.cpp
    sdmnbase/Downsampler.cpp
    sdmnbase/IntHalfbandFilter.cpp
    sdmnbase/Source.cpp
    sdmnbase/UDPSink.cpp
    sdmnbase/UDPSinkUncompressed.cpp
    sdmnbase/UDPSinkLZ4.cpp
    sdmnbase/UDPSocket.cpp
)

set(sdmnbase_HEADERS
    include/CRC64.h
    include/DataBuffer.h
    include/Decimators.h
    include/Downsampler.h
    include/IntHalfbandFilter.h
    include/parsekv.h
    include/Source.h
    include/UDPSink.h
    include/UDPSinkUncompressed.h
    include/UDPSinkLZ4.h
    include/UDPSocket.h
    include/util.h
)

set(sdmnfecbase_SOURCES
    sdmnbase/CRC64.cpp
    sdmnbase/Decimators.cpp
    sdmnbase/Downsampler.cpp
    sdmnbase/IntHalfbandFilter.cpp
    sdmnbase/Source.cpp
    sdmnbase/UDPSink.cpp
    sdmnbase/UDPSinkFEC.cpp
    sdmnbase/UDPSinkUncompressed.cpp
    sdmnbase/UDPSinkLZ4.cpp
    sdmnbase/UDPSocket.cpp
)

set(sdmnfecbase_HEADERS
    include/CRC64.h
    include/DataBuffer.h
    include/Decimators.h
    include/Downsampler.h
    include/IntHalfbandFilter.h
    include/parsekv.h
    include/Source.h
    include/UDPSink.h
    include/UDPSinkFEC.h
    include/UDPSinkUncompressed.h
    include/UDPSinkLZ4.h
    include/UDPSocket.h
    include/util.h
)

# Base sources

set(sdmnbase_SOURCES
    ${sdmnbase_SOURCES}
    ${sdmnbase_HEADERS}
)

set(sdmnfecbase_SOURCES
    ${sdmnfecbase_SOURCES}
    ${sdmnfecbase_HEADERS}
)

# Test sources

set(sdmntest_SOURCES
    sdmnbase/TestSource.cpp
)

set(sdmntest_HEADERS
    include/TestSource.h
)

# Libraries

add_library(sdmnbase STATIC
    ${sdmnbase_SOURCES}
)

add_library(sdmntest STATIC
    ${sdmntest_SOURCES}
)

add_executable(sdrdaemon
    sdrdaemon.cpp
)

if(USE_SIMD AND CM256CC_FOUND)
    add_library(sdmnfecbase STATIC
        ${sdmnfecbase_SOURCES}
    )

    add_executable(sdrdaemonfec
        sdrdaemonfec.cpp
    )
endif(USE_SIMD AND CM256CC_FOUND)

add_executable(sdrdmnctl
    sdrdmnctl.cpp
)

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${LIBNANOMSG_INCLUDE_DIR}
    ${CM256CC_INCLUDE_DIR}
    ${EXTRA_INCLUDES}
)

set(DEVICE_LIBRARIES "")
set(DEVICE_TARGETS "")

if(LIBUSB_FOUND AND LIBAIRSPY_FOUND)
    set(sdmnairspy_SOURCES
        sdmnbase/AirspySource.cpp
    )

    set(sdmnairspy_HEADERS
        include/AirspySource.h
    )

    add_library(sdmnairspy STATIC
        ${sdmnairspy_SOURCES}
    )

    target_include_directories(sdmnairspy PUBLIC
        ${LIBAIRSPY_INCLUDE_DIRS}
    )

    target_link_libraries(sdmnairspy
        ${LIBAIRSPY_LIBRARIES}
    )

    set(DEVICE_LIBRARIES ${DEVICE_LIBRARIES} sdmnairspy)
    set(DEVICE_TARGETS ${DEVICE_TARGETS} sdmnairspy)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHAS_AIRSPY")
endif(LIBUSB_FOUND AND LIBAIRSPY_FOUND)

if(LIBUSB_FOUND AND LIBBLADERF_FOUND)
    set(sdmnbladerf_SOURCES
        sdmnbase/BladeRFSource.cpp
    )

    set(sdmnbladerf_HEADERS
        include/BladeRFSource.h
    )

    add_library(sdmnbladerf STATIC
        ${sdmnbladerf_SOURCES}
    )

    target_include_directories(sdmnbladerf PUBLIC
        ${LIBBLADERF_INCLUDE_DIRS}
    )

    target_link_libraries(sdmnbladerf
        ${LIBBLADERF_LIBRARIES}
    )

    set(DEVICE_LIBRARIES ${DEVICE_LIBRARIES} sdmnbladerf)
    set(DEVICE_TARGETS ${DEVICE_TARGETS} sdmnbladerf)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHAS_BLADERF")
endif(LIBUSB_FOUND AND LIBBLADERF_FOUND)

if(LIBUSB_FOUND AND LIBHACKRF_FOUND)
    set(sdmnhackrf_SOURCES
        sdmnbase/HackRFSource.cpp
    )

    set(sdmnhackrf_HEADERS
        include/HackRFSource.h
    )

    add_library(sdmnhackrf STATIC
        ${sdmnhackrf_SOURCES}
    )

    target_include_directories(sdmnhackrf PUBLIC
        ${LIBHACKRF_INCLUDE_DIRS}
    )

    target_link_libraries(sdmnhackrf
        ${LIBHACKRF_LIBRARIES}
    )

    set(DEVICE_LIBRARIES ${DEVICE_LIBRARIES} sdmnhackrf)
    set(DEVICE_TARGETS ${DEVICE_TARGETS} sdmnhackrf)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHAS_HACKRF")
endif(LIBUSB_FOUND AND LIBHACKRF_FOUND)

if(LIBUSB_FOUND AND LIBRTLSDR_FOUND)
    set(sdmnrtlsdr_SOURCES
        sdmnbase/RtlSdrSource.cpp
    )

    set(sdmnrtlsdr_HEADERS
        include/RtlSdrSource.h
    )

    add_library(sdmnrtlsdr STATIC
        ${sdmnrtlsdr_SOURCES}
    )

    target_include_directories(sdmnrtlsdr PUBLIC
        ${LIBRTLSDR_INCLUDE_DIRS}
    )

    target_link_libraries(sdmnrtlsdr
        ${LIBRTLSDR_LIBRARIES}
    )

    set(DEVICE_LIBRARIES ${DEVICE_LIBRARIES} sdmnrtlsdr)
    set(DEVICE_TARGETS ${DEVICE_TARGETS} sdmnrtlsdr)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHAS_RTLSDR")
endif(LIBUSB_FOUND AND LIBRTLSDR_FOUND)

target_include_directories(sdmntest PUBLIC
    ${LIBTEST_INCLUDE_DIRS}
)

target_link_libraries(sdmntest
    ${LIBTEST_LIBRARIES}
)

target_link_libraries(sdmnbase ${LZ4_LIBRARIES})

target_link_libraries(sdrdaemon
    sdmnbase
    ${DEVICE_LIBRARIES}
    sdmntest
    ${CMAKE_THREAD_LIBS_INIT}
    ${EXTRA_LIBS}
)

if(USE_SIMD AND CM256CC_FOUND)
    target_link_libraries(sdmnfecbase ${LZ4_LIBRARIES} ${CM256CC_LIBRARIES})
    
    target_link_libraries(sdrdaemonfec
        sdmnfecbase
        ${DEVICE_LIBRARIES}
        sdmntest
        ${CMAKE_THREAD_LIBS_INIT}
        ${EXTRA_LIBS}
    )
endif(USE_SIMD AND CM256CC_FOUND)

target_include_directories(sdrdmnctl PUBLIC
    ${LIBNANOMSG_INCLUDE_DIR}
)

target_link_libraries(sdrdmnctl
    ${LIBNANOMSG_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${EXTRA_LIBS}
)

if(USE_SIMD AND CM256CC_FOUND)
    install(TARGETS sdrdaemon sdrdaemonfec sdrdmnctl DESTINATION bin)
    install(TARGETS sdmnbase sdmnfecbase ${DEVICE_TARGETS} sdmntest DESTINATION lib)
else(USE_SIMD AND CM256CC_FOUND)
    install(TARGETS sdrdaemon sdrdmnctl DESTINATION bin)
    install(TARGETS sdmnbase ${DEVICE_TARGETS} sdmntest DESTINATION lib)
endif(USE_SIMD AND CM256CC_FOUND)