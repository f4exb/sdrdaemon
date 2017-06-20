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
    m_outputMeta.init();
    m_paramsCM256.BlockBytes = sizeof(ProtectedBlock);
    m_paramsCM256.OriginalCount = nbOriginalBlocks;
    m_paramsCM256.RecoveryCount = -1;
    m_decoderIndexHead = nbDecoderSlots / 2;
    m_frameHead = -1;
    m_curNbBlocks = 0;
    m_curNbRecovery = 0;

    if (cm256_init()) {
        m_cm256_OK = false;
        std::cerr << "SDRdaemonFECBuffer::SDRdaemonFECBuffer: cannot initialize CM256 library" << std::endl;
    } else {
        m_cm256_OK = true;
    }
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

void SDRdaemonFECBuffer::getSlotData(uint8_t *data, uint32_t& dataLength)
{
    dataLength = (nbOriginalBlocks - 1) * samplesPerBlock * sizeof(Sample);
    memcpy((void *) data, (const void *) &m_decoderSlot.m_frame.m_blocks[1], dataLength); // skip block 0

    if (m_decoderSlot.m_metaRetrieved)
    {
        MetaDataFEC *metaData = (MetaDataFEC *) &m_decoderSlot.m_frame.m_blocks[0];

        if (!(*metaData == m_outputMeta))
        {
            m_outputMeta = *metaData;
        }
    }

    if (!m_decoderSlot.m_decoded)
    {
        std::cerr << "SDRdaemonFECBuffer::getSlotData: incomplete frame:"
                << " m_blockCount: " << m_decoderSlot.m_blockCount
                << " m_recoveryCount: " << m_decoderSlot.m_recoveryCount << std::endl;
    }
}

void SDRdaemonFECBuffer::initDecodeSlot()
{
    // collect stats before voiding the slot
    m_curNbBlocks = m_decoderSlot.m_blockCount;
    m_curNbRecovery = m_decoderSlot.m_recoveryCount;
    m_avgNbBlocks(m_curNbBlocks);
    m_avgNbRecovery(m_curNbRecovery);
    // void the slot
    m_decoderSlot.m_blockCount = 0;
    m_decoderSlot.m_recoveryCount = 0;
    m_decoderSlot.m_decoded = false;
    m_decoderSlot.m_metaRetrieved = false;
    memset((void *) &m_decoderSlot.m_frame, 0, sizeof(BufferFrame0));
}

bool SDRdaemonFECBuffer::writeAndRead(uint8_t *array, std::size_t length, uint8_t *data, uint32_t& dataLength)
{
    assert(length == udpSize);

    bool dataAvailable = false;
    dataLength = 0;
    SuperBlock *superBlock = (SuperBlock *) array;
    int frameIndex = superBlock->header.frameIndex;

//    std::cerr << "SDRdaemonFECBuffer::writeAndRead:"
//            << " frameIndex: " << frameIndex
//            << " decoderIndex: " << decoderIndex
//            << " blockIndex: " << blockIndex
//            << " i.q:";
//
//    for (int i = 0; i < 10; i++)
//    {
//        std::cerr << " " << (int) superBlock->protectedBlock.samples[i].i
//                << "." << (int) superBlock->protectedBlock.samples[i].q;
//    }
//
//    std::cerr << std::endl;

    if ( m_frameHead != frameIndex)
    {
        getSlotData(data, dataLength); // copy slot data to output buffer
        dataAvailable = true;
        initDecodeSlot(); // re-initialize slot
        m_frameHead = frameIndex;
    }

    // decoderIndex should now be correctly set

    if (m_decoderSlot.m_blockCount < nbOriginalBlocks) // not enough blocks to decode -> store data
    {
        int blockCount = m_decoderSlot.m_blockCount;
        int recoveryCount = m_decoderSlot.m_recoveryCount;
        int blockIndex = superBlock->header.blockIndex;
        m_decoderSlot.m_cm256DescriptorBlocks[blockCount].Index = blockIndex;

        if (blockIndex == 0) // first block with meta
        {
            m_decoderSlot.m_metaRetrieved = true;
        }

        if (blockIndex < nbOriginalBlocks) // data block
        {
            m_decoderSlot.m_frame.m_blocks[blockIndex] = superBlock->protectedBlock;
            m_decoderSlot.m_cm256DescriptorBlocks[blockCount].Block = (void *) &m_decoderSlot.m_frame.m_blocks[blockIndex];
        }
        else // redundancy block
        {
            m_decoderSlot.m_recoveryBlocks[recoveryCount] = superBlock->protectedBlock;
            m_decoderSlot.m_cm256DescriptorBlocks[blockCount].Block = (void *) &m_decoderSlot.m_recoveryBlocks[recoveryCount];
            m_decoderSlot.m_recoveryCount++;
        }
    }

    m_decoderSlot.m_blockCount++;

    if (m_decoderSlot.m_blockCount == nbOriginalBlocks) // ready to decode
    {
        m_decoderSlot.m_decoded = true;

        if (m_cm256_OK && (m_decoderSlot.m_recoveryCount > 0)) // recovery data used and CM256 decoder available
        {
            m_paramsCM256.RecoveryCount = m_decoderSlot.m_recoveryCount;
//            // debug print
//            for (int ir = 0; ir < m_decoderSlots[decoderIndex].m_recoveryCount; ir++) // recovery blocks
//            {
//                int blockIndex = m_decoderSlots[decoderIndex].m_cm256DescriptorBlocks[nbRxOriginalBlocks+ir].Index;
//                ProtectedBlock *recoveryBlock = (ProtectedBlock *) m_decoderSlots[decoderIndex].m_cm256DescriptorBlocks[nbRxOriginalBlocks+ir].Block;
//
//                std::cerr << "SDRdaemonFECBuffer::writeAndRead:"
//                        << " recovery block #" << blockIndex
//                        << " i.q: ";
//
//                for (int i = 0; i < 10; i++)
//                {
//                    std::cerr << " " << recoveryBlock->samples[i].i
//                            << "." << recoveryBlock->samples[i].q;
//                }
//
//                std::cerr << std::endl;
//            }
//            // end debug print

            if (cm256_decode(m_paramsCM256, m_decoderSlot.m_cm256DescriptorBlocks)) // failure to decode
            {
                std::cerr << "SDRdaemonFECBuffer::writeAndRead: CM256 decode error" << std::endl;
            }
            else // success to decode
            {
                int nbRxOriginalBlocks = nbOriginalBlocks - m_decoderSlot.m_recoveryCount;

                std::cerr << "SDRdaemonFECBuffer::writeAndRead: CM256 decode success:"
                        << " nb recovery blocks: " << m_decoderSlot.m_recoveryCount << std::endl;

                for (int ir = 0; ir < m_decoderSlot.m_recoveryCount; ir++) // recover lost blocks
                {
                    int recoveryIndex = nbOriginalBlocks - m_decoderSlot.m_recoveryCount + ir;
                    int blockIndex = m_decoderSlot.m_cm256DescriptorBlocks[recoveryIndex].Index;
                    ProtectedBlock *recoveredBlock = (ProtectedBlock *) m_decoderSlot.m_cm256DescriptorBlocks[recoveryIndex].Block;
                    m_decoderSlot.m_frame.m_blocks[blockIndex] =  *recoveredBlock;

//                    if (blockIndex == 0)
//                    {
//                        m_decoderSlots[decoderIndex].m_metaRetrieved = true;
//                    }

                    // debug print
                    std::cerr << "SDRdaemonFECBuffer::writeAndRead:"
                            << " recovered block #" << blockIndex
                            << " i.q: ";

                    for (int i = 0; i < 10; i++)
                    {
                        std::cerr << " " << recoveredBlock->samples[i].i
                                << "." << recoveredBlock->samples[i].q;
                    }

                    std::cerr << std::endl;
                    // end debug print
                } // recover
            } // success to decode
        } // recovery data used

        if (m_decoderSlot.m_metaRetrieved) // meta data retrieved
        {
            MetaDataFEC *metaData = (MetaDataFEC *) &m_decoderSlot.m_frame.m_blocks[0];

            if (!(*metaData == m_currentMeta))
            {
                m_currentMeta = *metaData;
                printMeta(metaData);
            }
        }
    } // decode frame

    return dataAvailable;
}
