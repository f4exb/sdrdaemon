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

void SDRdaemonFECBuffer::initDecodeAllSlots()
{
    for (int i = 0; i < nbDecoderSlots; i++)
    {
        m_decoderSlots[i].m_blockCount = 0;
        m_decoderSlots[i].m_recoveryCount = 0;
        m_decoderSlots[i].m_decoded = false;
        m_decoderSlots[i].m_metaRetrieved = false;
        m_decoderSlots[i].m_blockZero.m_metaData.init();
    }
}

void SDRdaemonFECBuffer::getSlotData(int slotIndex, uint8_t *data, std::size_t& dataLength)
{
    dataLength = (nbOriginalBlocks - 1) * samplesPerBlock * sizeof(Sample);
    memcpy((void *) data, (const void *) &m_frames[slotIndex].m_blocks, dataLength);

    if (!m_decoderSlots[slotIndex].m_decoded)
    {
        std::cerr << "SDRdaemonFECBuffer::getSlotData: incomplete frame:"
                << " m_blockCount: " << m_decoderSlots[slotIndex].m_blockCount
                << " m_recoveryCount: " << m_decoderSlots[slotIndex].m_recoveryCount << std::endl;
    }

    if (m_decoderSlots[slotIndex].m_blockZero.m_metaData.m_nbFECBlocks >= 0) // valid meta
    {
        m_outputMeta = m_decoderSlots[slotIndex].m_blockZero.m_metaData; // store it
    }
}

void SDRdaemonFECBuffer::initDecodeSlot(int slotIndex)
{
    // collect stats before voiding the slot
    m_curNbBlocks = m_decoderSlots[slotIndex].m_blockCount;
    m_curNbRecovery = m_decoderSlots[slotIndex].m_recoveryCount;
    m_avgNbBlocks(m_curNbBlocks);
    m_avgNbRecovery(m_curNbRecovery);
    // void the slot
    m_decoderSlots[slotIndex].m_blockCount = 0;
    m_decoderSlots[slotIndex].m_recoveryCount = 0;
    m_decoderSlots[slotIndex].m_decoded = false;
    m_decoderSlots[slotIndex].m_metaRetrieved = false;
    m_decoderSlots[slotIndex].m_blockZero.m_metaData.init();
    memset((void *) m_frames[slotIndex].m_blocks, 0, (nbOriginalBlocks - 1) * samplesPerBlock * sizeof(Sample));
}

bool SDRdaemonFECBuffer::writeAndRead(uint8_t *array, std::size_t length, uint8_t *data, std::size_t& dataLength)
{
    assert(length == udpSize);

    bool dataAvailable = false;
    dataLength = 0;
    SuperBlock *superBlock = (SuperBlock *) array;
    int frameIndex = superBlock->header.frameIndex;
    int decoderIndex = frameIndex % nbDecoderSlots;
    int blockIndex = superBlock->header.blockIndex;

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

    if (m_frameHead == -1) // initial state
    {
        m_decoderIndexHead = decoderIndex; // new decoder slot head
        m_frameHead = frameIndex;
        initDecodeAllSlots(); // initialize all slots
    }
    else
    {
        int frameDelta = m_frameHead - frameIndex;

        if (frameDelta < 0)
        {
            if (-frameDelta < nbDecoderSlots) // new frame head not too new
            {
                m_decoderIndexHead = decoderIndex; // new decoder slot head
                m_frameHead = frameIndex;
                getSlotData(decoderIndex, data, dataLength); // copy slot data to output buffer
                dataAvailable = true;
                initDecodeSlot(decoderIndex); // re-initialize current slot
            }
            else if (-frameDelta <= sizeof(uint16_t) - nbDecoderSlots) // loss of sync start over
            {
                m_decoderIndexHead = decoderIndex; // new decoder slot head
                m_frameHead = frameIndex;
                initDecodeAllSlots(); // re-initialize all slots
            }
        }
        else
        {
            if (frameDelta > sizeof(uint16_t) - nbDecoderSlots) // new frame head not too new
            {
                m_decoderIndexHead = decoderIndex; // new decoder slot head
                m_frameHead = frameIndex;
                getSlotData(decoderIndex, data, dataLength); // copy slot data to output buffer
                dataAvailable = true;
                initDecodeSlot(decoderIndex); // re-initialize current slot
            }
            else if (frameDelta >= nbDecoderSlots) // loss of sync start over
            {
                m_decoderIndexHead = decoderIndex; // new decoder slot head
                m_frameHead = frameIndex;
                initDecodeAllSlots(); // re-initialize all slots
            }
        }
    }

    // decoderIndex should now be correctly set

    int blockCount = m_decoderSlots[decoderIndex].m_blockCount;
    int recoveryCount = m_decoderSlots[decoderIndex].m_recoveryCount;

    if (blockCount < nbOriginalBlocks) // not enough blocks to decode -> store data
    {
        if (blockIndex == 0) // first block with meta
        {
            ProtectedBlockZero *blockZero = (ProtectedBlockZero *) &superBlock->protectedBlock;
            m_decoderSlots[decoderIndex].m_blockZero = *blockZero;
            m_decoderSlots[decoderIndex].m_cm256DescriptorBlocks[blockCount].Block = (void *) &m_decoderSlots[decoderIndex].m_blockZero;
            m_decoderSlots[decoderIndex].m_metaRetrieved = true;
        }
        else if (blockIndex < nbOriginalBlocks) // normal block
        {
            m_frames[decoderIndex].m_blocks[blockIndex - 1] = superBlock->protectedBlock;
            m_decoderSlots[decoderIndex].m_cm256DescriptorBlocks[blockCount].Block = (void *) &m_frames[decoderIndex].m_blocks[blockIndex - 1];
        }
        else // redundancy block
        {
            m_decoderSlots[decoderIndex].m_recoveryBlocks[recoveryCount] = superBlock->protectedBlock;
            m_decoderSlots[decoderIndex].m_cm256DescriptorBlocks[blockCount].Block = (void *) &m_decoderSlots[decoderIndex].m_recoveryBlocks[recoveryCount];
            m_decoderSlots[decoderIndex].m_recoveryCount++;
        }

        m_decoderSlots[decoderIndex].m_cm256DescriptorBlocks[blockCount].Index = blockIndex;
    }

    m_decoderSlots[decoderIndex].m_blockCount++;

    if (m_decoderSlots[decoderIndex].m_blockCount == nbOriginalBlocks) // ready to decode
    {
        m_decoderSlots[decoderIndex].m_decoded = true;

        if (m_decoderSlots[decoderIndex].m_recoveryCount > 0) // recovery data used
        {
//            if (m_decoderSlots[decoderIndex].m_metaRetrieved) // block zero has been received
//            {
//                m_paramsCM256.RecoveryCount = m_decoderSlots[decoderIndex].m_blockZero.m_metaData.m_nbFECBlocks;
//            }
//            else
//            {
//                m_paramsCM256.RecoveryCount = m_currentMeta.m_nbFECBlocks; // take last value for number of FEC blocks
//            }

            m_paramsCM256.RecoveryCount = m_decoderSlots[decoderIndex].m_recoveryCount;
            int nbRxOriginalBlocks = nbOriginalBlocks - m_decoderSlots[decoderIndex].m_recoveryCount;

//            // debug print
//            for (int ir = 0; ir < m_decoderSlots[decoderIndex].m_recoveryCount; ir++) // recovery blocks
//            {
//                int blockIndex = m_decoderSlots[decoderIndex].m_cm256DescriptorBlocks[nbRxOriginalBlocks+ir].Index;
//                std::cerr << "SDRdaemonFECBuffer::writeAndRead:"
//                        << " recovery block #" << blockIndex
//                        << " i.q: ";
//
//                for (int i = 0; i < 10; i++)
//                {
//                    std::cerr << " " << m_decoderSlots[decoderIndex].m_recoveryBlocks[ir].samples[i].i
//                            << "." << m_decoderSlots[decoderIndex].m_recoveryBlocks[ir].samples[i].q;
//                }
//
//                std::cerr << std::endl;
//            }
//            // end debug print

            if (cm256_decode(m_paramsCM256, m_decoderSlots[decoderIndex].m_cm256DescriptorBlocks)) // failure to decode
            {
                std::cerr << "SDRdaemonFECBuffer::writeAndRead: CM256 decode error" << std::endl;
            }
            else // success to decode
            {
                std::cerr << "SDRdaemonFECBuffer::writeAndRead: CM256 decode success:"
                        << " nb recovery blocks: " << m_decoderSlots[decoderIndex].m_recoveryCount << std::endl;

                for (int ir = 0; ir < m_decoderSlots[decoderIndex].m_recoveryCount; ir++) // recover lost blocks
                {
                    int blockIndex = m_decoderSlots[decoderIndex].m_cm256DescriptorBlocks[nbRxOriginalBlocks+ir].Index;
                    ProtectedBlock *recoveredBlock = (ProtectedBlock *) m_decoderSlots[decoderIndex].m_cm256DescriptorBlocks[nbRxOriginalBlocks+ir].Block;

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

                    if (blockIndex == 0)
                    {
                        ProtectedBlockZero *recoveredBlockZero = (ProtectedBlockZero *) recoveredBlock;
                        m_decoderSlots[decoderIndex].m_blockZero.m_metaData = recoveredBlockZero->m_metaData;
                        m_decoderSlots[decoderIndex].m_metaRetrieved = true;
                    }
                    else
                    {
                        m_frames[decoderIndex].m_blocks[blockIndex - 1] =  *recoveredBlock;
                    }
                }
            } // success to decode
        } // recovery data used

        if (m_decoderSlots[decoderIndex].m_metaRetrieved) // meta data retrieved
        {
            if (!(m_decoderSlots[decoderIndex].m_blockZero.m_metaData == m_currentMeta))
            {
                printMeta(&m_decoderSlots[decoderIndex].m_blockZero.m_metaData); // print for change other than timestamp
            }

            m_currentMeta = m_decoderSlots[decoderIndex].m_blockZero.m_metaData; // renew current meta
        }
    } // decode frame

    return dataAvailable;
}
