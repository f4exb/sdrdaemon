# CMake definitions for sdrdaemon

cmake_minimum_required(VERSION 3.0.2)
project(sdrdaemon)

find_package(Threads)
find_package(PkgConfig)
find_package(ALSA REQUIRED)
find_package(Boost 1.47)

# Find RTL-SDR library.
pkg_check_modules(PKG_RTLSDR librtlsdr)
find_path(RTLSDR_INCLUDE_DIR rtl-sdr.h
          HINT ${PKG_RTLSDR_INCLUDE_DIRS})
find_library(RTLSDR_LIBRARY librtlsdr.a
             HINT ${PKG_RTLSDR_LIBRARY_DIRS})

# Find HackRF library.
pkg_check_modules(PKG_HACKRF libhackrf)
find_path(HACKRF_INCLUDE_DIR hackrf.h
          HINT ${PKG_HACKRF_INCLUDE_DIRS})
find_library(HACKRF_LIBRARY libhackrf.a
             HINT ${PKG_HACKRF_LIBRARY_DIRS})

# Find Airspy library.
pkg_check_modules(PKG_AIRSPY libairspy)
find_path(AIRSPY_INCLUDE_DIR airspy.h
          HINT ${PKG_AIRSPY_INCLUDE_DIRS})
find_library(AIRSPY_LIBRARY libairspy.a
             HINT ${PKG_AIRSPY_LIBRARY_DIRS})

# Find BladeRF library.
pkg_check_modules(PKG_BLADERF libbladerf)
find_path(BLADERF_INCLUDE_DIR libbladeRF.h
          HINT ${PKG_BLADERF_INCLUDE_DIRS})
find_library(BLADERF_LIBRARY libbladeRF.so
             HINT ${PKG_BLADERF_LIBRARY_DIRS})

# Find libusb
pkg_check_modules(PKG_LIBUSB libusb-1.0)
find_path(LIBUSB_INCLUDE_DIR libusb.h
          HINT ${PKG_LIBUSB_INCLUDE_DIRS}
          PATH_SUFFIXES libusb-1.0)
find_library(LIBUSB_LIBRARY usb-1.0
             HINT ${PKG_LIBUSB_LIBRARY_DIRS})

if(RTLSDR_INCLUDE_DIR AND RTLSDR_LIBRARY)
    message(STATUS "Found librtlsdr: ${RTLSDR_INCLUDE_DIR}, ${RTLSDR_LIBRARY}")
else()
    message(WARNING "Can not find Osmocom RTL-SDR library")
    message("Try again with environment variable PKG_CONFIG_PATH")
    message("or with -DRTLSDR_INCLUDE_DIR=/path/rtlsdr/include")
    message("        -DRTLSDR_LIBRARY=/path/rtlsdr/lib/librtlsdr.a")
endif()

set(RTLSDR_INCLUDE_DIRS ${RTLSDR_INCLUDE_DIR} ${LIBUSB_INCLUDE_DIR})
set(RTLSDR_LIBRARIES    ${RTLSDR_LIBRARY} ${LIBUSB_LIBRARY})

set(HACKRF_INCLUDE_DIRS ${HACKRF_INCLUDE_DIR} ${LIBUSB_INCLUDE_DIR})
set(HACKRF_LIBRARIES    ${HACKRF_LIBRARY} ${LIBUSB_LIBRARY})

set(AIRSPY_INCLUDE_DIRS ${AIRSPY_INCLUDE_DIR} ${LIBUSB_INCLUDE_DIR})
set(AIRSPY_LIBRARIES    ${AIRSPY_LIBRARY} ${LIBUSB_LIBRARY})

set(BLADERF_INCLUDE_DIRS ${BLADERF_INCLUDE_DIR} ${LIBUSB_INCLUDE_DIR})
set(BLADERF_LIBRARIES    ${BLADERF_LIBRARY} ${LIBUSB_LIBRARY})

# Compiler flags.
set(CMAKE_CXX_FLAGS "-Wall -std=c++11 -O2 -ffast-math -ftree-vectorize ${EXTRA_FLAGS}")

set(sdmnbase_SOURCES
    sdmnbase/Filter.cpp
    sdmnbase/Downsampler.cpp
    sdmnbase/UDPSink.cpp
    sdmnbase/UDPSocket.cpp
)

set(sdmnbase_HEADERS
    include/Filter.h
    include/Downsampler.h
    include/UDPSink.h
    include/UDPSocket.h
    include/MovingAverage.h
    include/Source.h
    include/SDRDaemon.h
    include/DataBuffer.h
    include/parsekv.h
    include/util.h
)

# Base sources

set(sdmnbase_SOURCES
    ${sdmnbase_SOURCES}
    ${sdmnbase_HEADERS}
)

# RTL-SDR sources

set(sdmnrtlsdr_SOURCES
    sdmnbase/RtlSdrSource.cpp
)

set(sdmnrtlsdr_HEADERS
    include/RtlSdrSource.h
)

set(sdmnrtlsdr_SOURCES
    ${sdmnrtlsdr_SOURCES}
    ${sdmnrtlsdr_HEADERS}
)

# HackRF sources

set(sdmnhackrf_SOURCES
    sdmnbase/HackRFSource.cpp
)

set(sdmnhackrf_HEADERS
    include/HackRFSource.h
)

set(sdmnhackrf_SOURCES
    ${sdmnhackrf_SOURCES}
    ${sdmnhackrf_HEADERS}
)

# Airspy sources

set(sdmnairspy_SOURCES
    sdmnbase/AirspySource.cpp
)

set(sdmnairspy_HEADERS
    include/AirspySource.h
)

set(sdmnairspy_SOURCES
    ${sdmnairspy_SOURCES}
    ${sdmnairspy_HEADERS}
)

# BLadeRF sources

set(sdmnbladerf_SOURCES
    sdmnbase/BladeRFSource.cpp
)

set(sdmnbladerf_HEADERS
    include/BladeRFSource.h
)

set(sdmnbladerf_SOURCES
    ${sdmnbladerf_SOURCES}
    ${sdmnbladerf_HEADERS}
)

# Libraries

add_library(sdmnbase STATIC
    ${sdmnbase_SOURCES}
)

add_library(sdmnrtlsdr STATIC
    ${sdmnrtlsdr_SOURCES}
)

add_library(sdmnhackrf STATIC
    ${sdmnhackrf_SOURCES}
)

add_library(sdmnairspy STATIC
    ${sdmnairspy_SOURCES}
)

add_library(sdmnbladerf STATIC
    ${sdmnbladerf_SOURCES}
)

add_executable(sdrdaemon
	main.cpp
)

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${ALSA_INCLUDE_DIRS}
    ${EXTRA_INCLUDES} 
)

target_link_libraries(sdrdaemon
    sdmnbase
    sdmnrtlsdr
    sdmnhackrf
    sdmnairspy
    sdmnbladerf
    ${CMAKE_THREAD_LIBS_INIT}
    ${ALSA_LIBRARIES}
    ${EXTRA_LIBS} 
)

target_include_directories(sdmnrtlsdr PUBLIC
    ${RTLSDR_INCLUDE_DIRS}
)

target_link_libraries(sdmnrtlsdr
    ${RTLSDR_LIBRARIES}
)

target_link_libraries(sdmnhackrf
    ${HACKRF_LIBRARIES}
)

target_link_libraries(sdmnairspy
    ${AIRSPY_LIBRARIES}
)

target_link_libraries(sdmnbladerf
    ${BLADERF_LIBRARIES}
)

install(TARGETS sdrdaemon DESTINATION bin)
install(TARGETS sdmnbase sdmnrtlsdr sdmnhackrf sdmnairspy sdmnbladerf DESTINATION lib)
