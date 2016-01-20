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

#include <sys/time.h>
#include <iostream>

#include "UDPSinkUncompressed.h"

UDPSinkUncompressed::UDPSinkUncompressed(const std::string& address, unsigned int port, unsigned int udpSize) :
		UDPSink(address, port, udpSize)
{
}

UDPSinkUncompressed::~UDPSinkUncompressed()
{
}

void UDPSinkUncompressed::write(const IQSampleVector& samples_in)
{
	MetaData *metaData = (MetaData *) m_bufMeta;
    struct timeval tv;
    uint16_t samplesPerBlock = m_udpSize / (2 * m_sampleBytes);
    uint32_t nbCompleteBlocks = samples_in.size() / samplesPerBlock;
    uint32_t remainderSamples = samples_in.size() % samplesPerBlock;

    gettimeofday(&tv, 0);

    metaData->m_centerFrequency = m_centerFrequency;
    metaData->m_sampleRate = m_sampleRate;
    metaData->m_sampleBytes = m_sampleBytes & 0x0F;
    metaData->m_sampleBits = m_sampleBits;
    metaData->m_blockSize = m_udpSize;
    metaData->m_nbSamples = samples_in.size();
    metaData->m_nbBlocks = 1;
    metaData->m_nbBytes = samples_in.size() * 2 * m_sampleBytes;
    metaData->m_tv_sec = tv.tv_sec;
    metaData->m_tv_usec = tv.tv_usec;
	metaData->m_crc = m_crc64.calculate_crc(m_bufMeta, sizeof(MetaData) - 8);

	if (!(*metaData == m_currentMeta))
	{
		std::cerr << "UDPSinkUncompressed::write: meta: "
				<< "|" << metaData->m_centerFrequency
				<< ":" << metaData->m_sampleRate
				<< ":" << (int) (metaData->m_sampleBytes & 0xF)
				<< ":" << (int) metaData->m_sampleBits
				<< ":" << metaData->m_blockSize
				<< ":" << metaData->m_nbSamples
				<< "||" << metaData->m_nbBlocks
				<< ":" << metaData->m_nbBytes
				<< "|" << metaData->m_tv_sec
				<< ":" << metaData->m_tv_usec
				<< " " << remainderSamples
				<< ":" << nbCompleteBlocks
				<< " samplesPerBlock: " << samplesPerBlock
				<< std::endl;

		m_currentMeta = *metaData;
	}

	m_socket.SendDataGram((const void *) m_bufMeta, (int) m_udpSize, m_address, m_port);

	for (unsigned int i = 0; i < nbCompleteBlocks; i++)
	{
		m_socket.SendDataGram((const void *) &samples_in[i*samplesPerBlock], (int) m_udpSize, m_address, m_port);
	}

	if (remainderSamples > 0)
	{
		memcpy((void *) m_buf, (const void *) &samples_in[nbCompleteBlocks*samplesPerBlock], 2 * remainderSamples * (metaData->m_sampleBytes & 0xF));
		m_socket.SendDataGram((const void *) m_buf, (int) m_udpSize, m_address, m_port);
	}
}
