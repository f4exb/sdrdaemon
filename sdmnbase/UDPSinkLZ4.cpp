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

#include "UDPSinkLZ4.h"

UDPSinkLZ4::UDPSinkLZ4(const std::string& address, unsigned int port, unsigned int udpSize, unsigned int lz4MinInputSize) :
		UDPSink(address, port, udpSize),
		m_lz4MinInputSize(lz4MinInputSize),
		m_lz4HardBLockSize(0),
		m_lz4MaxInputBlocks(0),
		m_lz4MaxInputSize(0),
		m_lz4InputBlockCount(0),
		m_lz4BufSize(0),
		m_lz4InputCount(0),
		m_lz4Count(0),
		m_lz4InputBuffer(0),
		m_lz4Buffer(0),
		m_lz4Stream(0)
{
	m_lz4Meta.init();
	m_lz4Stream = LZ4_createStream();
}

UDPSinkLZ4::~UDPSinkLZ4()
{
	LZ4_freeStream(m_lz4Stream);

    if (m_lz4InputBuffer) {
    	delete[] m_lz4InputBuffer;
    }

	if (m_lz4Buffer) {
		delete[] m_lz4Buffer;
	}
}

void UDPSinkLZ4::write(const IQSampleVector& samples_in)
{
	MetaData *metaData = (MetaData *) m_bufMeta;
    struct timeval tv;

    gettimeofday(&tv, 0);

    metaData->m_centerFrequency = m_centerFrequency;
    metaData->m_sampleRate = m_sampleRate;
    metaData->m_sampleBytes = 0x10 + (m_sampleBytes & 0x0F); // add LZ4 indicator
    metaData->m_sampleBits = m_sampleBits;
    metaData->m_blockSize = m_udpSize;
    metaData->m_nbSamples = samples_in.size();
    metaData->m_tv_sec = tv.tv_sec;
    metaData->m_tv_usec = tv.tv_usec;
	metaData->m_crc = m_crc64.calculate_crc((uint8_t *) m_bufMeta, sizeof(MetaData) - 8);

	if (m_lz4MaxInputBlocks && (m_lz4InputBlockCount == m_lz4MaxInputBlocks)) // send previous data
    {
		// update meta
		m_lz4Meta.m_nbBlocks = m_lz4InputBlockCount;
		m_lz4Meta.m_nbBytes = m_lz4Count;
		m_lz4Meta.m_crc = m_crc64.calculate_crc((uint8_t *) &m_lz4Meta, sizeof(MetaData) - 8); // recalculate CRC

		//std::cerr << "UDPSinkLZ4::write: frame complete: ";
		//printMeta(&m_lz4Meta);

		udpSend(); // output data to UDP

		LZ4_resetStream(m_lz4Stream);
		m_lz4InputBlockCount = 0;
		m_lz4Count = 0;
		m_lz4InputCount = 0;
		m_lz4Meta = *metaData; // store for future use
    }

	if (!(*metaData == m_currentMeta)) // If critical meta data has changed send previous data and update current meta
	{
		if (m_lz4Count == 0) // nothing to write from the LZ4 buffer
		{
			m_lz4Meta = *metaData; // store for future use
            setLZ4Values(metaData->m_nbSamples, m_sampleBytes);
            std::cerr << "UDPSinkLZ4::write: new meta: no data: "
            << m_lz4MaxInputBlocks
            << ":" << m_lz4BufSize
            << " ";
            printMeta(&m_lz4Meta);
		}
		else
		{
			// update meta
			m_lz4Meta.m_nbBlocks = m_lz4InputBlockCount;
			m_lz4Meta.m_nbBytes = m_lz4Count;
			m_lz4Meta.m_crc = m_crc64.calculate_crc((uint8_t *) &m_lz4Meta, sizeof(MetaData) - 8); // recalculate CRC

			std::cerr << "UDPSinkLZ4::write: new meta: "
            << m_lz4MaxInputBlocks
            << ":" << m_lz4BufSize
            << " ";
            printMeta(&m_lz4Meta);

			udpSend(); // output data to UDP

			LZ4_resetStream(m_lz4Stream);
			m_lz4InputBlockCount = 0;
			m_lz4Count = 0;
			m_lz4InputCount = 0;
			m_lz4Meta = *metaData; // store for future use

			// Calculate new LZ4 values if relevant sizes have changed
			if ((metaData->m_nbSamples != m_currentMeta.m_nbSamples) || (m_sampleBytes != (m_currentMeta.m_sampleBytes & 0x0F)))
			{
                setLZ4Values(metaData->m_nbSamples, m_sampleBytes);
			}
		}

		m_currentMeta = *metaData;
	}

	// process hardware block (compress)
	memcpy((void *) &m_lz4InputBuffer[m_lz4InputCount], (const void *) &samples_in[0], m_lz4HardBLockSize);
	int compressedBytes = LZ4_compress_fast_continue(m_lz4Stream, (const char *) &m_lz4InputBuffer[m_lz4InputCount], (char *) &m_lz4Buffer[m_lz4Count], samples_in.size() * 2 * m_sampleBytes, m_lz4BufSize, 1);
	m_lz4InputCount += m_lz4HardBLockSize;
	m_lz4Count += compressedBytes;
	m_lz4InputBlockCount++;
}

void UDPSinkLZ4::udpSend()
{
	uint32_t nbCompleteBlocks = m_lz4Count / m_udpSize;
	uint32_t remainderBytes = m_lz4Count % m_udpSize;

	m_socket.SendDataGram((const void *) &m_lz4Meta, (int) m_udpSize, m_address, m_port);

	for (unsigned int i = 0; i < nbCompleteBlocks; i++)
	{
		m_socket.SendDataGram((const void *) &m_lz4Buffer[i*m_udpSize], (int) m_udpSize, m_address, m_port);
	}

	if (remainderBytes > 0)
	{
		memcpy((void *) m_buf, (const void *) &m_lz4Buffer[nbCompleteBlocks*m_udpSize], remainderBytes);
		m_socket.SendDataGram((const void *) m_buf, (int) m_udpSize, m_address, m_port);
	}
}

void UDPSinkLZ4::printMeta(MetaData *metaData)
{
	std::cerr
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
			<< std::endl;
}

void UDPSinkLZ4::setLZ4Values(uint32_t nbSamples, uint8_t sampleBytes)
{
    m_lz4HardBLockSize = nbSamples * 2 *sampleBytes;
    m_lz4MaxInputBlocks = (m_lz4MinInputSize / m_lz4HardBLockSize) + 1;
    m_lz4MaxInputSize = m_lz4MaxInputBlocks * m_lz4HardBLockSize;
    m_lz4BufSize = LZ4_compressBound(m_lz4MaxInputSize);

    std::cerr << "UDPSinkLZ4::setLZ4Values:"
    		<< " m_lz4HardBLockSize: " << m_lz4HardBLockSize
			<< ", m_lz4MaxInputBlocks: " << m_lz4MaxInputBlocks
			<< ", m_lz4MaxInputSize: " << m_lz4MaxInputSize
			<< ", m_lz4BufSize: " << m_lz4BufSize
			<< std::endl;

    if (m_lz4InputBuffer) {
    	delete[] m_lz4InputBuffer;
    }

    m_lz4InputBuffer = new uint8_t[m_lz4MaxInputSize];

    if (m_lz4Buffer) {
        delete[] m_lz4Buffer;
    }

    m_lz4Buffer = new uint8_t[m_lz4BufSize];
}
