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
# Include python install macros
########################################################################
include(GrPython)
if(NOT PYTHONINTERP_FOUND)
    return()
endif()

########################################################################
# Install python sources
########################################################################
GR_PYTHON_INSTALL(
    FILES
    __init__.py
    DESTINATION ${GR_PYTHON_DIR}/sdrdaemon
)

########################################################################
# Handle the unit tests
########################################################################
#include(GrTest)
#
#set(GR_TEST_TARGET_DEPS gnuradio-sdrdaemon)
#set(GR_TEST_PYTHON_DIRS ${CMAKE_BINARY_DIR}/swig)
#GR_ADD_TEST(qa_sdrdaemon ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/qa_sdrdaemon.py)

