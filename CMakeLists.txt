cmake_minimum_required (VERSION 3.0.2)
project (sdradaemon)

find_package(Threads)
find_package(PkgConfig)
find_package(Boost 1.47)

# Find Airspy library.
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

# Find HackRF library.
find_path(HACKRF_INCLUDE_DIR hackrf.h
          HINT ${PKG_HACKRF_INCLUDE_DIRS})
find_library(HACKRF_LIBRARY libhackrf.a
          HINT ${PKG_HACKRF_LIBRARY_DIRS})

# Find RTL-SDR library.
pkg_check_modules(PKG_RTLSDR librtlsdr)
find_path(RTLSDR_INCLUDE_DIR rtl-sdr.h
          HINT ${PKG_RTLSDR_INCLUDE_DIRS})
find_library(RTLSDR_LIBRARY librtlsdr.a
          HINT ${PKG_RTLSDR_LIBRARY_DIRS})

# Find libusb
pkg_check_modules(PKG_LIBUSB libusb-1.0)
find_path(LIBUSB_INCLUDE_DIR libusb.h
          HINT ${PKG_LIBUSB_INCLUDE_DIRS}
          PATH_SUFFIXES libusb-1.0)
find_library(LIBUSB_LIBRARY usb-1.0
          HINT ${PKG_LIBUSB_LIBRARY_DIRS})

set(AIRSPY_INCLUDE_DIRS ${AIRSPY_INCLUDE_DIR} ${LIBUSB_INCLUDE_DIR})
set(AIRSPY_LIBRARIES    ${AIRSPY_LIBRARY} ${LIBUSB_LIBRARY})

set(BLADERF_INCLUDE_DIRS ${BLADERF_INCLUDE_DIR} ${LIBUSB_INCLUDE_DIR})
set(BLADERF_LIBRARIES    ${BLADERF_LIBRARY} ${LIBUSB_LIBRARY})

set(HACKRF_INCLUDE_DIRS ${HACKRF_INCLUDE_DIR} ${LIBUSB_INCLUDE_DIR})
set(HACKRF_LIBRARIES    ${HACKRF_LIBRARY} ${LIBUSB_LIBRARY})

set(RTLSDR_INCLUDE_DIRS ${RTLSDR_INCLUDE_DIR} ${LIBUSB_INCLUDE_DIR})
set(RTLSDR_LIBRARIES    ${RTLSDR_LIBRARY} ${LIBUSB_LIBRARY})

# Compiler flags.
set(CMAKE_CXX_FLAGS "-Wall -std=c++11 -O2 -ffast-math -ftree-vectorize ${EXTRA_FLAGS}")

set(sdmnbase_SOURCES
    sdmnbase/Downsampler.cpp
    sdmnbase/Filter.cpp
    sdmnbase/UDPSink.cpp
    sdmnbase/UDPSocket.cpp
)

set(sdmnbase_HEADERS
    include/DataBuffer.h
    include/Downsampler.h
    include/Filter.h
    include/parsekv.h
    include/Source.h
    include/UDPSink.h
    include/UDPSocket.h
    include/util.h
)

# Base sources

set(sdmnbase_SOURCES
    ${sdmnbase_SOURCES}
    ${sdmnbase_HEADERS}
)

# Airspy sources

set(sdmnairspy_SOURCES
    sdmnbase/AirspySource.cpp
)

set(sdmnairspy_HEADERS
    include/AirspySource.h
)

# BladeRF sources

set(sdmnbladerf_SOURCES
    sdmnbase/BladeRFSource.cpp
)

set(sdmnbladerf_HEADERS
    include/BladeRFSource.h
)

# HackRF sources

set(sdmnhackrf_SOURCES
    sdmnbase/HackRFSource.cpp
)

set(sdmnhackrf_HEADERS
    include/HackRFSource.h
)

# RTL-SDR sources

set(sdmnrtlsdr_SOURCES
    sdmnbase/RtlSdrSource.cpp
)

set(sdmnrtlsdr_HEADERS
    include/RtlSdrSource.h
)

# Libraries

add_library(sdmnbase STATIC
    ${sdmnbase_SOURCES}
)

add_library(sdmnairspy STATIC
    ${sdmnairspy_SOURCES}
)

add_library(sdmnbladerf STATIC
    ${sdmnbladerf_SOURCES}
)

add_library(sdmnhackrf STATIC
    ${sdmnhackrf_SOURCES}
)

add_library(sdmnrtlsdr STATIC
    ${sdmnrtlsdr_SOURCES}
)

add_executable(sdrdaemon
    main.cpp
)

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${EXTRA_INCLUDES} 
)

target_link_libraries(sdrdaemon
    sdmnbase
    sdmnairspy
    sdmnbladerf
    sdmnhackrf
    sdmnrtlsdr
    ${CMAKE_THREAD_LIBS_INIT}
    ${EXTRA_LIBS} 
)

target_include_directories(sdmnairspy PUBLIC
    ${AIRSPY_INCLUDE_DIRS}
)

target_link_libraries(sdmnairspy
    ${AIRSPY_LIBRARIES}
)

target_include_directories(sdmnbladerf PUBLIC
    ${BLADERF_INCLUDE_DIRS}
)

target_link_libraries(sdmnbladerf
    ${BLADERF_LIBRARIES}
)

target_include_directories(sdmnhackrf PUBLIC
    ${HACKRF_INCLUDE_DIRS}
)

target_link_libraries(sdmnhackrf
    ${HACKRF_LIBRARIES}
)

target_include_directories(sdmnrtlsdr PUBLIC
    ${RTLSDR_INCLUDE_DIRS}
)

target_link_libraries(sdmnrtlsdr
    ${RTLSDR_LIBRARIES}
)

install(TARGETS sdrdaemon DESTINATION bin)
install(TARGETS sdmnbase sdmnairspy sdmnbladerf sdmnhackrf sdmnrtlsdr DESTINATION lib)
