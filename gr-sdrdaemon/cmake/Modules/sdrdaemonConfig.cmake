INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_SDRDAEMON sdrdaemon)

FIND_PATH(
    SDRDAEMON_INCLUDE_DIRS
    NAMES sdrdaemon/api.h
    HINTS $ENV{SDRDAEMON_DIR}/include
        ${PC_SDRDAEMON_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    SDRDAEMON_LIBRARIES
    NAMES gnuradio-sdrdaemon
    HINTS $ENV{SDRDAEMON_DIR}/lib
        ${PC_SDRDAEMON_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(SDRDAEMON DEFAULT_MSG SDRDAEMON_LIBRARIES SDRDAEMON_INCLUDE_DIRS)
MARK_AS_ADVANCED(SDRDAEMON_LIBRARIES SDRDAEMON_INCLUDE_DIRS)

