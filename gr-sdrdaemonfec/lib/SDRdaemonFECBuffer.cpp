///////////////////////////////////////////////////////////////////////////////////
// SDRdaemon - send I/Q samples read from a SDR device over the network via UDP. //
//             GNUradio interface.                                               //
//                                                                               //
// This is an adaptation of the GNUradio UDP source                              //
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

#include <cassert>
#include <cstring>
#include <iostream>

#include "SDRdaemonFECBuffer.h"

SDRdaemonFECBuffer::SDRdaemonFECBuffer()
{
	m_currentMeta.init();
	m_paramsCM256.BlockBytes = sizeof(ProtectedBlock);
	m_paramsCM256.OriginalCount = nbOriginalBlocks;
	m_paramsCM256.RecoveryCount = -1;
	m_decoderSlotHead = nbDecoderSlots / 2;
	m_frameHead = -1;
}

SDRdaemonFECBuffer::~SDRdaemonFECBuffer()
{
}

void SDRdaemonFECBuffer::printMeta(MetaDataFEC *metaData)
{
	std::cerr
			<< "|" << metaData->m_centerFrequency
			<< ":" << metaData->m_sampleRate
			<< ":" << (int) (metaData->m_sampleBytes & 0xF)
			<< ":" << (int) metaData->m_sampleBits
			<< ":" << (int) metaData->m_nbOriginalBlocks
			<< ":" << (int) metaData->m_nbFECBlocks
	        << "|" << metaData->m_tv_sec
			<< ":" << metaData->m_tv_usec
			<< "|" << std::endl;
}

bool SDRdaemonFECBuffer::writeAndRead(uint8_t *array, std::size_t length, uint8_t *data, std::size_t& dataLength)
{
    assert(length == udpSize);

    SuperBlock *superBlock = (SuperBlock *) array;
    int frameIndex = superBlock->header.frameIndex;

    if (frameIndex > m_frameHead) // new frame head
    {
        m_decoderSlotHead = (m_decoderSlotHead + (frameIndex - m_frameHead)) % nbDecoderSlots;
        m_frameHead = frameIndex;
    }

    if (frameIndex - m_frameHead > nbDecoderSlots) // frameIndex - m_frameHead in range ]-slots:0]
    {
        int decoderSlotIndex = (m_decoderSlotHead + frameIndex - m_frameHead) % nbDecoderSlots;
        return false;
    }
    else
    {
        return false;
    }
}
