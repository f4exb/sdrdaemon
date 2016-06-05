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
    m_nbDataSamples = ((UDPSINKFEC_UDPSIZE - 4) / 4);
    m_cm256Valid = (cm256_init() == 0);
}

UDPSinkFEC::~UDPSinkFEC()
{}

void UDPSinkFEC::write(const IQSampleVector& samples_in)
{
	MetaDataFEC metaData;
	struct timeval tv;
	IQSampleVector::const_iterator it = samples_in.begin();

	while (it != samples_in.end())
	{
		if (m_txBlockIndex == 0) // start of a new transmission frame
		{
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

			m_txBlocks[m_txBlockIndex].m_frameIndex = m_frameCount;
			m_txBlocks[m_txBlockIndex].m_blockIndex = 0;
			m_txBlocks[m_txBlockIndex].m_protectedBlock.m_blockIndex = 0;
			memcpy((void *) m_txBlocks[m_txBlockIndex].m_protectedBlock.m_data, (const void *) &metaData, sizeof(MetaDataFEC));

			m_sampleIndex = (sizeof(MetaDataFEC) / m_sampleBytes) + 1;
		}
		else if (m_txBlockIndex == UDPSINKFEC_NBORIGINALBLOCKS) // transmission frame is complete
		{
			transmitUDP(); // send the frame
			m_txBlockIndex = 0;
			continue;
		}

		int inSamplesIndex = it - samples_in.begin();
		int inRemainingSamples = samples_in.end() - it;

        if (m_sampleIndex  + inRemainingSamples < m_nbDataSamples) // there is still room in the current super block
        {
            // move all remaining samples to super block
            memcpy((void *) & m_txBlocks[m_txBlockIndex].m_protectedBlock.m_data[m_sampleIndex * m_sampleBytes],
                    (const void *) &samples_in[inSamplesIndex],
					inRemainingSamples * m_sampleBytes);
            it = samples_in.end(); // all input samples are consumed
            m_sampleIndex += inRemainingSamples;
        }
        else
        {
            // complete the super block
            memcpy((void *) & m_txBlocks[m_txBlockIndex].m_protectedBlock.m_data[m_sampleIndex * m_sampleBytes],
                    (const void *) &samples_in[inSamplesIndex],
                    (m_nbDataSamples - m_sampleIndex) * m_sampleBytes);
            // store in transmission frame
            m_txBlocks[m_txBlockIndex].m_blockIndex++;
            m_txBlocks[m_txBlockIndex].m_protectedBlock.m_blockIndex++;
            // advance in original blocks
            m_sampleIndex = 0;
            m_txBlockIndex++;
            // advance samples iterator
            it += m_nbDataSamples - m_sampleIndex;
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
			m_descriptorBlocks[i].Block = (void *) &(m_txBlocks[i].m_protectedBlock);
		}
		else
		{
			m_txBlocks[i].m_frameIndex = m_txBlocks[0].m_frameIndex; // same frame
			m_txBlocks[i].m_blockIndex = i;
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
		m_txBlocks[i+m_cm256Params.OriginalCount].m_protectedBlock = m_fecBlocks[i];
	}

    std::cerr << "UDPSinkFEC::transmitUDP: "
            << m_cm256Params.OriginalCount << " original blocks "
            << m_cm256Params.RecoveryCount << " recovery blocks" << std::endl;
}
