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
# Setup library
########################################################################
include(GrPlatform) #define LIB_SUFFIX
include(GrMiscUtils)

add_library(gnuradio-sdrdaemon SHARED
    sdrdmnsource_impl.cc
)
target_link_libraries(gnuradio-sdrdaemon ${Boost_LIBRARIES}  ${GNURADIO_ALL_LIBRARIES})
set_target_properties(gnuradio-sdrdaemon PROPERTIES DEFINE_SYMBOL "gnuradio_SDRDAEMON_EXPORTS")

########################################################################
# Install built library files
########################################################################
install(TARGETS gnuradio-sdrdaemon
    LIBRARY DESTINATION lib${LIB_SUFFIX} # .so/.dylib file
    ARCHIVE DESTINATION lib${LIB_SUFFIX} # .lib file
    RUNTIME DESTINATION bin              # .dll file
)

GR_LIBRARY_FOO(gnuradio-sdrdaemon)
