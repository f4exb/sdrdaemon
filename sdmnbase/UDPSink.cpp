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

#include <sys/time.h>
#include <iostream>

#include "UDPSink.h"

UDPSink::UDPSink(const std::string& address, unsigned int port, unsigned int udpSize) :
        m_address(address),
		m_port(port),
		m_udpSize(udpSize),
		m_centerFrequency(100000000),
		m_sampleRate(48000),
		m_sampleBytes(1),
		m_sampleBits(8)
{
	m_bufMeta = new uint8_t[m_udpSize];
	m_buf = new uint8_t[m_udpSize];
}

UDPSink::~UDPSink()
{
	delete[] m_buf;
	delete[] m_bufMeta;
}

void UDPSink::write(const IQSampleVector& samples_in)
{
	MetaData *metaData = (MetaData *) m_bufMeta;
    struct timeval tv;
    uint16_t samplesPerBlock = m_udpSize / (2 * m_sampleBytes);

    gettimeofday(&tv, 0);

    metaData->m_tv_sec = tv.tv_sec;
    metaData->m_tv_usec = tv.tv_usec;
    metaData->m_centerFrequency = m_centerFrequency;
    metaData->m_sampleRate = m_sampleRate;
    metaData->m_sampleBytes = m_sampleBytes;
    metaData->m_sampleBits = m_sampleBits;
    metaData->m_blockSize = m_udpSize;
    metaData->m_nbSamples = samples_in.size();
    metaData->m_remainderSamples = m_udpSize % (2 * m_sampleBytes);
    metaData->m_nbCompleteBlocks = samples_in.size() / samplesPerBlock;
	metaData->m_crc = m_crc64.calculate_crc(m_bufMeta, sizeof(MetaData) - 8);

	std::cerr << metaData->m_tv_sec
			<< ":" << metaData->m_tv_usec
			<< ":" << metaData->m_centerFrequency
			<< ":" << metaData->m_sampleRate
			<< ":" << (int) metaData->m_sampleBytes
			<< ":" << (int) metaData->m_sampleBits
			<< ":" << metaData->m_blockSize
			<< ":" << samples_in.size()
			<< ":(" << samplesPerBlock << ")"
			<< ":" << metaData->m_nbCompleteBlocks
			<< ":" << metaData->m_remainderSamples << std::endl;

	m_socket.SendDataGram((const void *) m_bufMeta, (int) m_udpSize, m_address, m_port);

	for (unsigned int i = 0; i < metaData->m_nbCompleteBlocks; i++)
	{
		m_socket.SendDataGram((const void *) &samples_in[i*samplesPerBlock], (int) m_udpSize, m_address, m_port);
	}

	if (metaData->m_remainderSamples > 0)
	{
		memcpy((void *) m_buf, (const void *) &samples_in[metaData->m_nbCompleteBlocks*samplesPerBlock], 2*metaData->m_remainderSamples*metaData->m_sampleBytes);
		m_socket.SendDataGram((const void *) m_buf, (int) m_udpSize, m_address, m_port);
	}

}
