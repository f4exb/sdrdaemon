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
#include "UDPSinkFEC.h"

UDPSinkFEC::UDPSinkFEC(const std::string& address, unsigned int port) :
    UDPSink::UDPSink(address, port, UDPSINKFEC_UDPSIZE),
    m_nbBlocksFEC(1),
	m_txBlockIndex(0),
	m_frameCount(0),
	m_sampleIndex(0)
{
    m_cm256Valid = (cm256_init() == 0);
    m_currentMetaFEC.init();
}

UDPSinkFEC::~UDPSinkFEC()
{}

void UDPSinkFEC::write(const IQSampleVector& samples_in)
{
	IQSampleVector::const_iterator it = samples_in.begin();

	while (it != samples_in.end())
	{
        int inSamplesIndex = it - samples_in.begin();
        int inRemainingSamples = samples_in.end() - it;

	    if (m_txBlockIndex == 0) // Tx block index 0 is a block with only meta data
	    {
            struct timeval tv;
            MetaDataFEC metaData;

            gettimeofday(&tv, 0);

            // create meta data TODO: semaphore
            metaData.m_centerFrequency = m_centerFrequency;
            metaData.m_sampleRate = m_sampleRate;
            metaData.m_sampleBytes = m_sampleBytes;
            metaData.m_sampleBits = m_sampleBits;
            metaData.m_nbOriginalBlocks = UDPSINKFEC_NBORIGINALBLOCKS;
            metaData.m_nbFECBlocks = m_nbBlocksFEC;
            metaData.m_tv_sec = tv.tv_sec;
            metaData.m_tv_usec = tv.tv_usec;

            memset((void *) &m_superBlock, 0, UDPSINKFEC_UDPSIZE);

            m_superBlock.header.frameIndex = m_frameCount;
            m_superBlock.header.blockIndex = 0;
            memcpy((void *) &m_superBlock.protectedBlock, (const void *) &metaData, sizeof(MetaDataFEC));

            if (!(metaData == m_currentMetaFEC))
            {
                std::cerr << "UDPSinkFEC::write: meta: "
                        << "|" << metaData.m_centerFrequency
                        << ":" << metaData.m_sampleRate
                        << ":" << (int) (metaData.m_sampleBytes & 0xF)
                        << ":" << (int) metaData.m_sampleBits
                        << "|" << (int) metaData.m_nbOriginalBlocks
                        << ":" << (int) metaData.m_nbFECBlocks
                        << "|" << metaData.m_tv_sec
                        << ":" << metaData.m_tv_usec
                        << "|" << std::endl;

                m_currentMetaFEC = metaData;
            }

            m_txBlocks[0] = m_superBlock;
            m_txBlockIndex = 1; // next Tx block with data
	    }

        if (m_sampleIndex + inRemainingSamples < samplesPerBlock) // there is still room in the current super block
        {
            memcpy((void *) &m_superBlock.protectedBlock.m_samples[m_sampleIndex],
                    (const void *) &samples_in[inSamplesIndex],
                    (samplesPerBlock - inRemainingSamples) * sizeof(IQSample));
            m_sampleIndex += inRemainingSamples;
            it = samples_in.end(); // all input samples are consumed
        }
        else // complete super block and initiate the next if not end of frame
        {
            memcpy((void *) &m_superBlock.protectedBlock.m_samples[m_sampleIndex],
                    (const void *) &samples_in[inSamplesIndex],
                    (samplesPerBlock - m_sampleIndex) * sizeof(IQSample));
            m_sampleIndex = 0;
            it += samplesPerBlock - m_sampleIndex;

            m_superBlock.header.frameIndex = m_frameCount;
            m_superBlock.header.blockIndex = m_txBlockIndex;
            m_txBlocks[m_txBlockIndex] =  m_superBlock;

            if (m_txBlockIndex == UDPSINKFEC_NBORIGINALBLOCKS - 1) // frame complete
            {
                transmitUDP();
                m_txBlockIndex = 0;
                m_frameCount++;
            }
            else
            {
                m_txBlockIndex++;
            }
        }
	}
}

void UDPSinkFEC::transmitUDP()
{
	if (!m_cm256Valid)
	{
		std::cerr << "UDPSinkFEC::transmitUDP: CM256 library initialization failure. No transmission." << std::endl;
		return;
	}

	int nbBlocksFEC = m_nbBlocksFEC; // do it once (atomic value)

	m_cm256Params.BlockBytes = sizeof(ProtectedBlock);
    m_cm256Params.OriginalCount = UDPSINKFEC_NBORIGINALBLOCKS;
    m_cm256Params.RecoveryCount = nbBlocksFEC;

    // Fill pointers to data
    for (int i = 0; i < m_cm256Params.OriginalCount + m_cm256Params.RecoveryCount; ++i)
    {
        if (i < m_cm256Params.OriginalCount)
        {
            m_descriptorBlocks[i].Block = (void *) &(m_txBlocks[i].protectedBlock);
        }
        else
        {
            m_txBlocks[i].header.frameIndex = m_frameCount;
            m_txBlocks[i].header.blockIndex = i;
        }
    }

    // Encode FEC blocks
    if (cm256_encode(m_cm256Params, m_descriptorBlocks, m_fecBlocks))
    {
        std::cerr << "UDPSinkFEC::transmitUDP: CM256 encode failed. No transmission." << std::endl;
        return;
    }

    // Merge FEC with data to transmit
    for (int i = 0; i < nbBlocksFEC; i++)
    {
        m_txBlocks[i + m_cm256Params.OriginalCount].protectedBlock = m_fecBlocks[i];
    }

    // Transmit all blocks
	for (int i = 0; i < m_cm256Params.OriginalCount + m_cm256Params.RecoveryCount; i++)
	{
//	    std::cerr << "UDPSinkFEC::transmitUDP:"
//	            << " i: " << i
//	            << " frameIndex: " << (int) m_txBlocks[i].header.frameIndex
//	            << " blockIndex: " << (int) m_txBlocks[i].header.blockIndex << std::endl;
	    m_socket.SendDataGram((const void *) &m_txBlocks[i], (int) m_udpSize, m_address, m_port);
	}
}
