///////////////////////////////////////////////////////////////////////////////////
// SDRdaemon - send I/Q samples read from a SDR device over the network via UDP. //
//                                                                               //
// Copyright (C) 2016 Edouard Griffiths, F4EXB                                   //
//                                                                               //
// This program is free software; you can redistribute it and/or modify          //
// it under the terms of the GNU General Public License as published by          //
// the Free Software Foundation as version 3 of the License, or                  //
//                                                                               //
// This program is distributed in the hope that it will be useful,               //
// but WITHOUT ANY WARRANTY; without even the implied warranty of                //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the                  //
// GNU General Public License V3 for more details.                               //
//                                                                               //
// You should have received a copy of the GNU General Public License             //
// along with this program. If not, see <http://www.gnu.org/licenses/>.          //
///////////////////////////////////////////////////////////////////////////////////

#include "UDPSinkFEC.h"

const int UDPSinkFEC::m_subFrameSize = UDPSINKFEC_SUBFRAMESIZE;
const int UDPSinkFEC::m_nbSubFrames = UDPSINKFEC_NBSUBFRAMES;
const int UDPSinkFEC::m_frameSize = UDPSINKFEC_NBSUBFRAMES * UDPSINKFEC_SUBFRAMESIZE;

UDPSinkFEC::UDPSinkFEC(const std::string& address, unsigned int port) :
    UDPSink::UDPSink(address, port, 513),
    m_nbBlocksFEC(1)
{
}

UDPSinkFEC::~UDPSinkFEC()
{}

void UDPSinkFEC::write(const IQSampleVector& samples_in)
{
    // TODO
}
