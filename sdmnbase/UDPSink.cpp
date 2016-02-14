///////////////////////////////////////////////////////////////////////////////////
// SDRdaemon - send I/Q samples read from a SDR device over the network via UDP. //
//                                                                               //
// Copyright (C) 2015 Edouard Griffiths, F4EXB                                   //
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

#include "UDPSink.h"

UDPSink::UDPSink(const std::string& address, unsigned int port, unsigned int udpSize) :
        m_address(address),
		m_port(port),
		m_udpSize(udpSize),
		m_centerFrequency(100000),
		m_sampleRate(48000),
		m_sampleBytes(1),
		m_sampleBits(8),
		m_nbSamples(0)
{
	m_currentMeta.init();
	m_bufMeta = new uint8_t[m_udpSize];
	m_buf = new uint8_t[m_udpSize];
}

UDPSink::~UDPSink()
{
	delete[] m_buf;
	delete[] m_bufMeta;
}
