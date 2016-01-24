///////////////////////////////////////////////////////////////////////////////////
// SDRdaemon - send I/Q samples read from a SDR device over the network via UDP. //
//             GNUradio interface.                                               //
//                                                                               //
// This is an adaptation of the GNUradio UDP source                              //
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

#include <cassert>
#include <cstring>
#include <iostream>
#include "SDRdaemonBuffer.h"

SDRdaemonBuffer::SDRdaemonBuffer(std::size_t blockSize) :
	m_blockSize(blockSize),
	m_sync(false),
	m_lz4(false),
	m_lz4InBuffer(0),
	m_lz4InCount(0),
	m_lz4InSize(0)
{
	m_buf = new uint8_t[blockSize];
}

SDRdaemonBuffer::~SDRdaemonBuffer()
{
    delete[] m_buf;
}

bool SDRdaemonBuffer::writeAndRead(uint8_t *array, std::size_t length, uint8_t *data, std::size_t& dataLength)
{
	assert(length == m_blockSize); // TODO: allow fragmented blocks with larger size
	MetaData *metaData = (MetaData *) array;

	if (m_crc64.calculate_crc(array, sizeof(MetaData) - 8) == metaData->m_crc)
	{
		dataLength = 0;
		std::cerr << "SDRdaemonBuffer::writeAndRead: ";
		printMeta(metaData);

		// sanity checks
		if (metaData->m_blockSize == m_blockSize) // sent blocksize matches given blocksize
		{
			if (metaData->m_sampleBytes & 0x10)
			{
				m_lz4 = true;
				updateSizes(metaData);
			}
			else
			{
				m_lz4 = false;
			}

			m_sync = true;
		}
		else
		{
			m_sync = false;
		}

		m_currentMeta = *metaData;
		return false;
	}
	else
	{
		if (m_sync)
		{
			if (m_lz4)
			{
				return writeAndReadLZ4(array, length, data, dataLength);
			}
			else
			{
				std::memcpy((void *) data, (const void *) array, length);
				dataLength = length;
				return true;
			}
		}
		else
		{
			dataLength = 0;
			return false;
		}
	}
}

bool SDRdaemonBuffer::writeAndReadLZ4(uint8_t *array, std::size_t length, uint8_t *data, std::size_t& dataLength)
{
    if (m_lz4InCount + length < m_lz4InSize)
    {
        std::memcpy((void *) &m_lz4InBuffer[m_lz4InCount], (const void *) array, length); // copy data in compressed Buffer
        dataLength = 0;
        m_lz4InCount += length;
    }
    else
    {
        std::memcpy((void *) &m_lz4InBuffer[m_lz4InCount], (const void *) array, m_lz4InSize - m_lz4InCount); // copy rest of data in compressed Buffer
    	std::memcpy((void *) data, (const void *) m_lz4InBuffer, m_lz4InSize); // send what is in buffer
    	dataLength = m_lz4InSize;
		m_lz4InCount = 0;
    }

    return dataLength != 0;
}

void SDRdaemonBuffer::updateSizes(MetaData *metaData)
{
	if (m_currentMeta.m_nbBytes != metaData->m_nbBytes)
	{
		m_lz4InSize = metaData->m_nbBytes;

		if (m_lz4InBuffer) {
			delete[] m_lz4InBuffer;
		}

		m_lz4InBuffer = new uint8_t[m_lz4InSize]; // provide extra space for a full UDP block
		m_lz4InCount = 0;
	}
}


void SDRdaemonBuffer::printMeta(MetaData *metaData)
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
