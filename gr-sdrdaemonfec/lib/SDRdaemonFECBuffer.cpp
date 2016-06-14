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
	initDecoderSlotsAddresses();
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

void SDRdaemonFECBuffer::initDecoderSlotsAddresses()
{
    for (int i = 0; i < nbDecoderSlots; i++)
    {
        for (int j = 0; j < nbOriginalBlocks - 1; j++)
        {
            m_decoderSlots[i].m_originalBlockPtrs[j] = &m_frames[i].m_blocks[j];
        }
    }
}

void SDRdaemonFECBuffer::initDecode()
{
    for (int i = 0; i < nbDecoderSlots; i++)
    {
        m_decoderSlots[i].m_blockCount = 0;
        m_decoderSlots[i].m_recoveryCount = 0;
        m_decoderSlots[i].m_blockZero.m_metaData.init();
    }
}

void SDRdaemonFECBuffer::getSlotDataAndStats(int slotIndex, uint8_t *data, std::size_t& dataLength)
{
	dataLength = samplesPerBlockZero*sizeof(Sample) + samplesPerBlock*(nbOriginalBlocks - 1)*sizeof(Sample);
	memcpy((void *) data, (const void *) &m_frames[slotIndex].m_blockZero, dataLength);
	// TODO: collect stats
}

void SDRdaemonFECBuffer::initDecodeSlot(int slotIndex)
{
    m_decoderSlots[slotIndex].m_blockCount = 0;
    m_decoderSlots[slotIndex].m_recoveryCount = 0;
    m_decoderSlots[slotIndex].m_blockZero.m_metaData.init();
    memset((void *) m_decoderSlots[slotIndex].m_blockZero.m_samples, 0, samplesPerBlockZero * sizeof(Sample));
    memset((void *) m_frames[slotIndex].m_blocks, 0, (nbOriginalBlocks - 1) * samplesPerBlock * sizeof(Sample));
}

bool SDRdaemonFECBuffer::writeAndRead(uint8_t *array, std::size_t length, uint8_t *data, std::size_t& dataLength)
{
    assert(length == udpSize);

    bool dataAvailable = false;
    SuperBlock *superBlock = (SuperBlock *) array;
    int frameIndex = superBlock->header.frameIndex;
    int decoderIndex;

    if (m_frameHead == -1) // initial state
    {
    	m_decoderSlotHead = frameIndex % nbDecoderSlots; // new decoder slot head
    	decoderIndex = m_decoderSlotHead;
    	m_frameHead = frameIndex;
        initDecode(); // initialize all slots
    }
    else
    {
    	int frameDelta = m_frameHead - frameIndex;

    	if (frameDelta < 0)
    	{
    		if (-frameDelta < nbDecoderSlots) // new frame head not too new
    		{
				m_decoderSlotHead = frameIndex % nbDecoderSlots; // new decoder slot head
				decoderIndex = m_decoderSlotHead;
				m_frameHead = frameIndex;
				getSlotDataAndStats(decoderIndex, data, dataLength); // copy slot data to output buffer
				initDecodeSlot(decoderIndex); // re-initialize current slot
    		}
    		else if (-frameDelta > sizeof(uint16_t) - nbDecoderSlots) // old frame not too old
    		{
    			decoderIndex = frameIndex % nbDecoderSlots;
    		}
    		else // loss of sync start over
    		{
				m_decoderSlotHead = frameIndex % nbDecoderSlots; // new decoder slot head
				decoderIndex = m_decoderSlotHead;
				m_frameHead = frameIndex;
				initDecode(); // re-initialize all slots
    		}
    	}
    	else
    	{
			if (frameDelta < nbDecoderSlots) // old frame not too old
			{
				decoderIndex = frameIndex % nbDecoderSlots;
			}
			else if (frameDelta > sizeof(uint16_t) - nbDecoderSlots) // new frame head not too new
			{
				m_decoderSlotHead = frameIndex % nbDecoderSlots; // new decoder slot head
				decoderIndex = m_decoderSlotHead;
				m_frameHead = frameIndex;
				getSlotDataAndStats(decoderIndex, data, dataLength); // copy slot data to output buffer
				initDecodeSlot(decoderIndex); // re-initialize current slot
			}
			else // loss of sync start over
			{
				m_decoderSlotHead = frameIndex % nbDecoderSlots; // new decoder slot head
				decoderIndex = m_decoderSlotHead;
				m_frameHead = frameIndex;
                initDecode(); // re-initialize all slots
			}
    	}
    }

    // decoderIndex should now be correctly set

    int blockIndex = superBlock->header.blockIndex;
    int blockHead = m_decoderSlots[decoderIndex].m_blockCount;

    if (blockHead < nbOriginalBlocks) // not enough blocks to decode -> store data
    {
        if (blockIndex == 0) // first block with meta
        {
            SuperBlockZero *superBlockZero = (SuperBlockZero *) array;
            m_decoderSlots[decoderIndex].m_blockZero = superBlockZero->protectedBlock;
            m_decoderSlots[decoderIndex].m_cm256DescriptorBlocks[blockHead].Block = (void *) &m_decoderSlots[decoderIndex].m_blockZero;
            memcpy((void *) m_frames[decoderIndex].m_blockZero.m_samples,
                    (const void *) m_decoderSlots[decoderIndex].m_blockZero.m_samples,
                    samplesPerBlockZero * sizeof(Sample));
        }
        else if (blockIndex < samplesPerBlock) // normal block
        {
            m_frames[decoderIndex].m_blocks[blockIndex - 1] = superBlock->protectedBlock;
            m_decoderSlots[decoderIndex].m_cm256DescriptorBlocks[blockHead].Block = (void *) &m_frames[decoderIndex].m_blocks[blockIndex - 1];
        }
        else // redundancy block
        {
            m_decoderSlots[decoderIndex].m_recoveryBlocks[m_decoderSlots[decoderIndex].m_recoveryCount] = superBlock->protectedBlock;
            m_decoderSlots[decoderIndex].m_recoveryCount++;
        }

        m_decoderSlots[decoderIndex].m_cm256DescriptorBlocks[blockHead].Index = blockIndex;
        m_decoderSlots[decoderIndex].m_blockCount++;

        return false;
    }
    else // ready to decode
    {
        if (m_decoderSlots[decoderIndex].m_blockZero.m_metaData.m_nbFECBlocks < 0) // block zero has not been received
        {
            m_paramsCM256.RecoveryCount = m_currentMeta.m_nbFECBlocks; // take last value for number of FEC blocks
        }
        else
        {
            m_paramsCM256.RecoveryCount = m_decoderSlots[decoderIndex].m_blockZero.m_metaData.m_nbFECBlocks;
        }

        if (m_decoderSlots[decoderIndex].m_recoveryCount > 0) // recovery data used
        {
            if (cm256_decode(m_paramsCM256, m_decoderSlots[decoderIndex].m_cm256DescriptorBlocks)) // failure to decode
            {
                std::cerr << "SDRdaemonFECBuffer::writeAndRead: CM256 decode error" << std::endl;
            }
            else // success to decode
            {
                int nbOriginalBlocks = m_decoderSlots[decoderIndex].m_blockCount - m_decoderSlots[decoderIndex].m_recoveryCount;

                for (int ir = 0; ir < m_decoderSlots[decoderIndex].m_recoveryCount; ir++) // recover lost blocks
                {
                    int blockIndex = m_decoderSlots[decoderIndex].m_cm256DescriptorBlocks[nbOriginalBlocks+ir].Index;

                    if (blockIndex == 0)
                    {
                        ProtectedBlockZero *recoveredBlockZero = (ProtectedBlockZero *) &m_decoderSlots[decoderIndex].m_recoveryBlocks[ir];
                        m_decoderSlots[decoderIndex].m_blockZero.m_metaData = recoveredBlockZero->m_metaData;
                        memcpy((void *) m_frames[decoderIndex].m_blockZero.m_samples,
                                (const void *) recoveredBlockZero->m_samples,
                                samplesPerBlockZero * sizeof(Sample));
                    }
                    else
                    {
                        m_frames[decoderIndex].m_blocks[blockIndex - 1] =  m_decoderSlots[decoderIndex].m_recoveryBlocks[ir];
                    }
                }
            }
        }

        if (m_decoderSlots[decoderIndex].m_blockZero.m_metaData.m_nbFECBlocks >= 0) // meta data valid
        {
            if (!(m_decoderSlots[decoderIndex].m_blockZero.m_metaData == m_currentMeta))
            {
                printMeta(&m_decoderSlots[decoderIndex].m_blockZero.m_metaData); // print for change other than timestamp
            }

            m_currentMeta = m_decoderSlots[decoderIndex].m_blockZero.m_metaData; // renew current meta
        }
    }

    return dataAvailable;
}
