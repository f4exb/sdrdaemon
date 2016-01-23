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
	m_lz4OutBuffer(0),
	m_lz4InCount(0),
	m_lz4OutCount(0),
	m_lz4InSize(0),
	m_lz4OutSize(0),
	m_lz4OutNbBlocks(2),
	m_lz4OutBlockCount(0),
	m_dataCount(0),
	m_blockCount(0)
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
		m_currentMeta = *metaData;
		dataLength = 0;

		// sanity checks
		if (metaData->m_blockSize == m_blockSize) // sent blocksize matches given blocksize
		{
			if (metaData->m_sampleBytes & 0x10)
			{
				m_lz4 = true;
				m_sync = setLZ4Values(); // undo sync if LZ4 values could not be initialized
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
	// send data if a block is ready
	if (m_blockCount != m_lz4OutBlockCount)
	{
		//std::size_t sendLength = 5*length;
		std::size_t sendLength = m_lz4OutSize;

	    if (m_dataCount + sendLength < m_lz4OutSize)
	    {
	    	std::memcpy((void *) data, (const void *) &m_lz4OutBuffer[m_blockCount*m_lz4OutSize], sendLength);
	    	dataLength = sendLength;
	    	m_dataCount += sendLength;
	    }
	    else
	    {
	    	std::memcpy((void *) data, (const void *) &m_lz4OutBuffer[m_blockCount*m_lz4OutSize], m_lz4OutSize - m_dataCount);
	    	dataLength = m_lz4OutSize - m_dataCount;
	    	m_blockCount = (++m_blockCount == m_lz4OutNbBlocks ? 0 : m_blockCount);
	    	m_dataCount = 0;
	    }
	}
	else
	{
		dataLength = 0;
	}

    if (m_lz4InCount + length < m_lz4InSize)
    {
        std::memcpy((void *) &m_lz4InBuffer[m_lz4InCount], (const void *) array, length); // copy data in compressed Buffer
        m_lz4InCount += length;
    }
    else
    {
        std::memcpy((void *) &m_lz4InBuffer[m_lz4InCount], (const void *) array, m_lz4InSize - m_lz4InCount); // copy rest of data in compressed Buffer
        m_lz4InCount += length; // exceed size so decompression will be activated
    }

    if (m_lz4InCount >= m_lz4InSize) // full input compressed block retrieved
    {
    	int bytesRead = LZ4_decompress_fast((const char*) m_lz4InBuffer, (char*) &m_lz4OutBuffer[m_lz4OutSize*m_lz4OutBlockCount], m_lz4OutSize);

    	if (bytesRead != m_lz4InSize)
    	{
    		std::cerr << "SDRdaemonBuffer::writeAndReadLZ4: decoding error:"
    				<< " read: " << bytesRead
					<< " expected: " << m_lz4InSize
					<< " out: " << m_lz4OutSize
					<< std::endl;
    	}

    	m_lz4OutBlockCount = (++m_lz4OutBlockCount == m_lz4OutNbBlocks ? 0 : m_lz4OutBlockCount);
    	m_lz4InCount = 0;
    }

    return dataLength != 0;
}

bool SDRdaemonBuffer::setLZ4Values()
{
	m_lz4InSize = m_currentMeta.m_nbBytes;
	uint32_t lz4OutSize = m_currentMeta.m_nbSamples * m_currentMeta.m_nbBlocks *  2 * (m_currentMeta.m_sampleBytes & 0x0F);
	m_lz4InCount = 0;
	m_lz4OutCount = 0;

	if (lz4OutSize != m_lz4OutSize) // new sizes
	{
		uint32_t lz4MaxInputSize = LZ4_compressBound(lz4OutSize);

		if (m_lz4InBuffer) {
			delete[] m_lz4InBuffer;
		}

		m_lz4InBuffer = new uint8_t[lz4MaxInputSize + m_blockSize]; // provide extra space for a full UDP block

		if (m_lz4OutBuffer) {
			delete[] m_lz4OutBuffer;
		}

		m_lz4OutBuffer = new uint8_t[lz4OutSize * m_lz4OutNbBlocks];
		m_lz4OutSize = lz4OutSize;
		m_lz4OutBlockCount = 0;
	}

	return true;
}
