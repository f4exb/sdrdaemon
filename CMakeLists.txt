cmake_minimum_required (VERSION 3.0.2)
project (sdradaemon)

find_package(Threads)
find_package(PkgConfig)
find_package(Boost 1.47)

# Find Airspy library.
pkg_check_modules(PKG_AIRSPY libairspy)
find_path(AIRSPY_INCLUDE_DIR airspy.h
          HINT ${PKG_AIRSPY_INCLUDE_DIRS})
find_library(AIRSPY_LIBRARY libairspy.a
             HINT ${PKG_AIRSPY_LIBRARY_DIRS})