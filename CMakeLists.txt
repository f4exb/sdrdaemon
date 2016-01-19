cmake_minimum_required (VERSION 3.0.2)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)

project (sdrdaemon)

find_package(Threads)
find_package(PkgConfig)
find_package(Boost 1.47)
find_package(ZeroMQ)
find_package(LZ4)

# Find Airspy library.
pkg_check_modules(PKG_AIRSPY libairspy)
find_path(LIBAIRSPY_INCLUDE_DIR airspy.h
          HINT ${PKG_LIBAIRSPY_INCLUDE_DIRS})
find_library(LIBAIRSPY_LIBRARIES libairspy.a
          HINT ${PKG_LIBAIRSPY_LIBRARIES_DIRS})

# Find BladeRF library.
pkg_check_modules(PKG_BLADERF libbladerf)
find_path(LIBBLADERF_INCLUDE_DIR libbladeRF.h
          HINT ${PKG_LIBBLADERF_INCLUDE_DIR})
find_library(LIBBLADERF_LIBRARIES libbladeRF.so
          HINT ${PKG_LIBBLADERF_LIBRARIES_DIRS})

# Find HackRF library.
pkg_check_modules(PKG_HACKRF libhackrf)
find_path(LIBHACKRF_INCLUDE_DIR hackrf.h
          HINT ${PKG_LIBHACKRF_INCLUDE_DIRS})
find_library(LIBHACKRF_LIBRARIES libhackrf.a
          HINT ${PKG_LIBHACKRF_LIBRARIES_DIRS})

# Find RTL-SDR library.
pkg_check_modules(PKG_RTLSDR librtlsdr)
find_path(LIBRTLSDR_INCLUDE_DIR rtl-sdr.h
          HINT ${PKG_LIBRTLSDR_INCLUDE_DIRS})
find_library(LIBRTLSDR_LIBRARIES librtlsdr.a
          HINT ${PKG_LIBRTLSDR_LIBRARIES_DIRS})

# Find libusb
pkg_check_modules(PKG_LIBUSB libusb-1.0)
find_path(LIBUSB_INCLUDE_DIR libusb.h
          HINT ${PKG_LIBUSB_INCLUDE_DIRS}
          PATH_SUFFIXES libusb-1.0)
find_library(LIBUSB_LIBRARY usb-1.0
          HINT ${PKG_LIBUSB_LIBRARY_DIRS})

set(LIBAIRSPY_INCLUDE_DIRS ${LIBAIRSPY_INCLUDE_DIR} ${LIBUSB_INCLUDE_DIR})
set(LIBAIRSPY_LIBRARIES    ${LIBAIRSPY_LIBRARIES} ${LIBUSB_LIBRARY} ${ZEROMQ_LIBRARIES})

set(LIBBLADERF_INCLUDE_DIRS ${LIBBLADERF_INCLUDE_DIR} ${LIBUSB_INCLUDE_DIR})
set(LIBBLADERF_LIBRARIES    ${LIBBLADERF_LIBRARIES} ${LIBUSB_LIBRARY} ${ZEROMQ_LIBRARIES})

set(LIBHACKRF_INCLUDE_DIRS ${LIBHACKRF_INCLUDE_DIR} ${LIBUSB_INCLUDE_DIR})
set(LIBHACKRF_LIBRARIES    ${LIBHACKRF_LIBRARIES} ${LIBUSB_LIBRARY} ${ZEROMQ_LIBRARIES})

set(LIBRTLSDR_INCLUDE_DIRS ${LIBRTLSDR_INCLUDE_DIR} ${LIBUSB_INCLUDE_DIR})
set(LIBRTLSDR_LIBRARIES    ${LIBRTLSDR_LIBRARIES} ${LIBUSB_LIBRARY} ${ZEROMQ_LIBRARIES})

# Compiler flags.
set(CMAKE_CXX_FLAGS "-Wall -std=c++11 -O2 -ffast-math -ftree-vectorize ${EXTRA_FLAGS}")

set(sdmnbase_SOURCES
    sdmnbase/CRC64.cpp
    sdmnbase/Decimators.cpp
    sdmnbase/Downsampler.cpp
    sdmnbase/IntHalfbandFilter.cpp
    sdmnbase/Source.cpp
    sdmnbase/UDPSink.cpp
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
    ${LIBAIRSPY_INCLUDE_DIRS}
)

target_link_libraries(sdmnairspy
    ${LIBAIRSPY_LIBRARIES}
)

target_include_directories(sdmnbladerf PUBLIC
    ${LIBBLADERF_INCLUDE_DIRS}
)

target_link_libraries(sdmnbladerf
    ${LIBBLADERF_LIBRARIES}
)

target_include_directories(sdmnhackrf PUBLIC
    ${LIBHACKRF_INCLUDE_DIRS}
)

target_link_libraries(sdmnhackrf
    ${LIBHACKRF_LIBRARIES}
)

target_include_directories(sdmnrtlsdr PUBLIC
    ${LIBRTLSDR_INCLUDE_DIRS}
)

target_link_libraries(sdmnrtlsdr
    ${LIBRTLSDR_LIBRARIES}
)

target_link_libraries(sdmnbase ${LZ4_LIBRARIES})

install(TARGETS sdrdaemon DESTINATION bin)
install(TARGETS sdmnbase sdmnairspy sdmnbladerf sdmnhackrf sdmnrtlsdr DESTINATION lib)
