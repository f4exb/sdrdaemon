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

UDPSinkLZ4::UDPSinkLZ4(const std::string& address, unsigned int port, unsigned int udpSize, unsigned int minFrameSize) :
		UDPSink(address, port, udpSize),
		m_minFrameSize(minFrameSize),
		m_hardBlockSize(0),
		m_maxInputBlocks(0),
		m_inputBlockCount(0),
		m_inputBuffer(0),
		m_maxOutputSize(0),
		m_outputBuffer(0)
{
	m_currentMeta.init();
}

UDPSinkLZ4::~UDPSinkLZ4()
{
	if (m_inputBuffer) {
		delete[] m_inputBuffer;
	}

	if (m_outputBuffer) {
		delete[] m_outputBuffer;
	}
}

void UDPSinkLZ4::write(const IQSampleVector& samples_in)
{
	MetaData *metaData = (MetaData *) m_bufMeta;
    struct timeval tv;

    gettimeofday(&tv, 0);

    metaData->m_centerFrequency = m_centerFrequency;
    metaData->m_sampleRate = m_sampleRate;
    metaData->m_sampleBytes = m_sampleBytes;
    metaData->m_sampleBits = m_sampleBits;
    metaData->m_blockSize = m_udpSize;
    metaData->m_nbSamples = samples_in.size();
    metaData->m_tv_sec = tv.tv_sec;
    metaData->m_tv_usec = tv.tv_usec;

	if (!(m_currentMeta == *metaData))
	{
		if (m_inputBlockCount > 0) // if some data is in buffer send it with the old meta data
		{
			m_sendMeta = m_currentMeta;
			m_sendMeta.m_sampleBytes = 0x10 + (m_sendMeta.m_sampleBytes & 0x0F); // add LZ4 indicator
			m_sendMeta.m_nbBlocks = m_inputBlockCount;
			m_sendMeta.m_nbBytes = compressInput();
			m_sendMeta.m_crc = m_crc64.calculate_crc((uint8_t *) &m_sendMeta, sizeof(MetaData) - 8);

			udpSend();

			m_inputBlockCount = 0;
		}

		if ((m_currentMeta.m_nbSamples != metaData->m_nbSamples) || (m_currentMeta.m_sampleBytes != metaData->m_sampleBytes))
		{
			updateSizes(metaData);
		}

		std::cerr << "UDPSinkLZ4::write: changed meta: ";
		printMeta(metaData);

		m_currentMeta = *metaData;
	}
	else if (m_inputBlockCount == m_maxInputBlocks) // input buffer is full so send it
	{
		m_sendMeta = m_currentMeta;
		m_sendMeta.m_sampleBytes = 0x10 + (m_sendMeta.m_sampleBytes & 0x0F); // add LZ4 indicator
		m_sendMeta.m_nbBlocks = m_inputBlockCount;
		m_sendMeta.m_nbBytes = compressInput();
		m_sendMeta.m_crc = m_crc64.calculate_crc((uint8_t *) &m_sendMeta, sizeof(MetaData) - 8);

		udpSend();

		m_inputBlockCount = 0;
	}

	memcpy((void *) &m_inputBuffer[m_inputBlockCount*m_hardBlockSize], (const void *) &samples_in[0], m_hardBlockSize);
	m_inputBlockCount++;
}

void UDPSinkLZ4::udpSend()
{
	uint32_t nbCompleteBlocks = m_sendMeta.m_nbBytes / m_udpSize;
	uint32_t nbRemainderBytes = m_sendMeta.m_nbBytes % m_udpSize;

	m_socket.SendDataGram((const void *) &m_sendMeta, (int) m_udpSize, m_address, m_port);

	for (unsigned int i = 0; i < nbCompleteBlocks; i++)
	{
		m_socket.SendDataGram((const void *) &m_outputBuffer[i*m_udpSize], (int) m_udpSize, m_address, m_port);
	}

	if (nbRemainderBytes > 0)
	{
		memcpy((void *) m_buf, (const void *) &m_outputBuffer[nbCompleteBlocks*m_udpSize], nbRemainderBytes);
		m_socket.SendDataGram((const void *) m_buf, (int) m_udpSize, m_address, m_port);
	}
}

void UDPSinkLZ4::updateSizes(MetaData *metaData)
{
	m_hardBlockSize = metaData->m_nbSamples * 2 * metaData->m_sampleBytes;
	m_maxInputBlocks = (m_minFrameSize / m_hardBlockSize) + 1;

	std::cerr << "UDPSinkLZ4::updateSizes:"
			<< " m_hardBlockSize: "  << m_hardBlockSize
			<< " m_maxInputBlocks: " << m_maxInputBlocks
			<< std::endl;

	if (m_inputBuffer) {
		delete[] m_inputBuffer;
	}

	m_inputBuffer = new uint8_t[m_hardBlockSize * m_maxInputBlocks];

	m_maxOutputSize = LZ4_compressBound(m_hardBlockSize * m_maxInputBlocks);

	if (m_outputBuffer) {
		delete[] m_outputBuffer;
	}

	m_outputBuffer = new uint8_t[m_maxOutputSize];
}

uint32_t UDPSinkLZ4::compressInput()
{
	uint32_t compSize  = LZ4_compress((const char *) m_inputBuffer, (char *) m_outputBuffer, m_hardBlockSize * m_inputBlockCount);
//	uint32_t compSizeU = LZ4_decompress_fast((const char*) m_outputBuffer, (char*) m_inputBuffer, m_hardBlockSize * m_inputBlockCount);
//
//	if (compSize != compSizeU)
//	{
//		std::cerr << "UDPSinkLZ4::compressInput: error"
//			<< " compSize: " << compSize
//			<< " compSizeU: " << compSizeU
//			<< std::endl;
//	}

	return compSize;
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
