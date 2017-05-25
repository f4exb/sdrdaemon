///////////////////////////////////////////////////////////////////////////////////
// SDRdaemon - receive I/Q samples over the network via UDP and write to a       //
//             SDR device .                                                      //
//                                                                               //
// Copyright (C) 2017 Edouard Griffiths, F4EXB                                   //
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

#include "UDPSource.h"

UDPSource::UDPSource(const std::string& address, unsigned int port, unsigned int udpSize) :
        m_address(address),
		m_port(port),
		m_udpSize(udpSize),
		m_sampleBytes(1),
		m_sampleBits(8),
		m_nbSamples(0)
{
	m_currentMeta.init();
	m_bufMeta = new uint8_t[m_udpSize];
	m_buf = new uint8_t[m_udpSize];
}

UDPSource::~UDPSource()
{
	delete[] m_buf;
	delete[] m_bufMeta;
}
