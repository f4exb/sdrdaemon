#
# SDRdaemon - send I/Q samples read from a SDR device over the network via UDP. 
#             GNUradio interface.                                               
#                                                                               
# Copyright (C) 2015 Edouard Griffiths, F4EXB                                   
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

########################################################################
# Include swig generation macros
########################################################################
find_package(SWIG)
find_package(PythonLibs)
if(NOT SWIG_FOUND OR NOT PYTHONLIBS_FOUND)
    return()
endif()
include(GrSwig)
include(GrPython)

set(GR_SWIG_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/
    ${CMAKE_CURRENT_BINARY_DIR}/../include/
)

########################################################################
# Setup swig generation
########################################################################
foreach(incdir ${GNURADIO_RUNTIME_INCLUDE_DIRS})
    list(APPEND GR_SWIG_INCLUDE_DIRS ${incdir}/gnuradio/swig)
endforeach(incdir)

set(GR_SWIG_LIBRARIES gnuradio-sdrdaemon)
set(GR_SWIG_DOC_FILE ${CMAKE_CURRENT_BINARY_DIR}/sdrdaemon_swig_doc.i)
set(GR_SWIG_DOC_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/sdrdaemon/
    ${CMAKE_CURRENT_BINARY_DIR}/../include/sdrdaemon/
)

GR_SWIG_MAKE(sdrdaemon_swig sdrdaemon_swig.i)

########################################################################
# Install the build swig module
########################################################################
GR_SWIG_INSTALL( TARGETS sdrdaemon_swig DESTINATION ${GR_PYTHON_DIR}/sdrdaemon)

########################################################################
# Install swig .i files for development
########################################################################
install(
    FILES
    sdrdaemon_swig.i
    ${CMAKE_CURRENT_BINARY_DIR}/sdrdaemon_swig_doc.i
    DESTINATION ${GR_INCLUDE_DIR}/sdrdaemon/swig
)
