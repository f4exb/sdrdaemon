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

#ifndef GR_SDRDAEMON_LIB_SDRDAEMONBUFFER_H_
#define GR_SDRDAEMON_LIB_SDRDAEMONBUFFER_H_

#include <stdint.h>
#include <cstddef>
#include "CRC64.h"


class SDRdaemonBuffer
{
public:
#pragma pack(push, 1)
	struct MetaData
	{
		uint32_t m_tv_sec;            //!< 04 seconds of timestamp at start time of frame processing
		uint32_t m_tv_usec;           //!< 08 microseconds of timestamp at start time of frame processing
		uint64_t m_centerFrequency;   //!< 16 center frequency in Hz
		uint32_t m_sampleRate;        //!< 20 sample rate in Hz
		uint8_t  m_sampleBytes;       //!< 21 number of bytes per sample + MSB: remainder sent first in meta block
		uint8_t  m_sampleBits;        //!< 22 number of effective bits per sample
		uint16_t m_samplesPerBlock;   //!< 24 number of I/Q samples per block
		uint16_t m_remainderSamples;  //!< 26 number of remainder I/Q samples
		uint16_t m_nbCompleteBlocks;  //!< 28 number of blocks full of samples
		uint64_t m_crc;               //!< 36 64 bit CRC of the above
	};
#pragma pack(pop)

	SDRdaemonBuffer(std::size_t blockSize);
	~SDRdaemonBuffer();
	bool writeAndRead(uint8_t *array, std::size_t length, uint8_t *data, std::size_t& dataLength);
	const MetaData& getCurrentMeta() const { return m_currentMeta; }

private:
	std::size_t m_blockSize;
	bool m_sync;
	MetaData m_currentMeta;
	CRC64 m_crc64;
	uint8_t *m_buf;
};

#endif /* GR_SDRDAEMON_LIB_SDRDAEMONBUFFER_H_ */
