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
#include "SDRdaemonBuffer.h"

SDRdaemonBuffer::SDRdaemonBuffer(std::size_t blockSize) :
	m_blockSize(blockSize),
	m_sync(false)
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
			if (metaData->m_sampleBytes * metaData->m_samplesPerBlock <  metaData->m_blockSize) {
				m_sync = true;
			} else {
				m_sync = false;
			}
		}
		else {
			m_sync = false;
		}

		return false;
	}
	else
	{
		if (m_sync)
		{
			std::memcpy((void *) data, (const void *) array, length);
			dataLength = length;
			return true;
		}
		else
		{
			dataLength = 0;
			return false;
		}
	}
}

