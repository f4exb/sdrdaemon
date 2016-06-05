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

set(GR_SWIG_LIBRARIES gnuradio-sdrdaemonfec)
set(GR_SWIG_DOC_FILE ${CMAKE_CURRENT_BINARY_DIR}/sdrdaemonfec_swig_doc.i)
set(GR_SWIG_DOC_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/sdrdaemonfec/
    ${CMAKE_CURRENT_BINARY_DIR}/../include/sdrdaemonfec/
)

GR_SWIG_MAKE(sdrdaemonfec_swig sdrdaemonfec_swig.i)

########################################################################
# Install the build swig module
########################################################################
GR_SWIG_INSTALL( TARGETS sdrdaemonfec_swig DESTINATION ${GR_PYTHON_DIR}/sdrdaemonfec)

########################################################################
# Install swig .i files for development
########################################################################
install(
    FILES
    sdrdaemonfec_swig.i
    ${CMAKE_CURRENT_BINARY_DIR}/sdrdaemonfec_swig_doc.i
    DESTINATION ${GR_INCLUDE_DIR}/sdrdaemonfec/swig
)
