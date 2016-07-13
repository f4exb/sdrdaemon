///////////////////////////////////////////////////////////////////////////////////
// SDRdaemon - send I/Q samples read from a SDR device over the network via UDP. //
//                                                                               //
// This version uses feclib for FEC encoding.                                    //
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
#include <unistd.h>
#include <iostream>
#include <thread>
#include "feclib/fec.h"
#include "UDPSinkFEC2.h"

UDPSinkFEC2::UDPSinkFEC2(const std::string& address, unsigned int port) :
    UDPSink::UDPSink(address, port, UDPSINKFEC_UDPSIZE),
    m_nbBlocksFEC(0),
    m_txThread(0),
	m_txBlockIndex(0),
	m_txBlocksIndex(0),
	m_frameCount(0),
	m_sampleIndex(0)
{
	FEClib::fec_init();
    m_currentMetaFEC.init();
    m_udpSent.store(true);
}

UDPSinkFEC2::~UDPSinkFEC2()
{
	if (m_txThread)
	{
		m_txThread->join();
		delete m_txThread;
	}
}

void UDPSinkFEC2::setTxDelay(int txDelay)
{
    std::cerr << "UDPSinkFEC::setTxDelay: txDelay: " << txDelay << std::endl;
    m_txDelay = txDelay;
}

void UDPSinkFEC2::setNbBlocksFEC(int nbBlocksFEC)
{
    std::cerr << "UDPSinkFEC::setNbBlocksFEC: nbBlocksFEC: " << nbBlocksFEC << std::endl;
    m_nbBlocksFEC = nbBlocksFEC;
}

void UDPSinkFEC2::write(const IQSampleVector& samples_in)
{
	IQSampleVector::const_iterator it = samples_in.begin();
	//std::cerr << "UDPSinkFEC::write: samples_in.size() = " << samples_in.size() << std::endl;

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
            m_superBlock.header.blockIndex = m_txBlockIndex;
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

            m_txBlocks[m_txBlocksIndex][0] = m_superBlock;
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
            m_txBlocks[m_txBlocksIndex][m_txBlockIndex] =  m_superBlock;

            if (m_txBlockIndex == UDPSINKFEC_NBORIGINALBLOCKS - 1) // frame complete
            {
                if (m_txThread)
                {
                	if (!m_udpSent.load())
                	{
                		std::cerr << "UDPSinkFEC::write: warning UDP transmission not finished" << std::endl;
                	}

                    m_txThread->join();
                    delete m_txThread;
                }

                int nbBlocksFEC = m_nbBlocksFEC;
                int txDelay = m_txDelay;

                m_txThread = new std::thread(transmitUDP, this, m_txBlocks[m_txBlocksIndex], m_frameCount, nbBlocksFEC, txDelay);
                //transmitUDP(this, m_txBlocks[m_txBlocksIndex], m_frameCount, m_nbBlocksFEC, m_txDelay, m_cm256Valid);

                m_txBlocksIndex = (m_txBlocksIndex + 1) % 4;
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

void UDPSinkFEC2::transmitUDP(UDPSinkFEC2 *udpSinkFEC, SuperBlock *txBlockx, uint16_t frameIndex, int nbBlocksFEC, int txDelay)
{
	ProtectedBlock fecBlocks[256];     //!< FEC data
    unsigned char *txDataBlocks[256];  //!< Pointers to original data blocks
    unsigned char *txFecBlocks[256];   //!< Pointers to FEC data blocks

//    std::cerr << "UDPSinkFEC::transmitUDP:"
//            << " nbBlocksFEC: " << nbBlocksFEC
//            << " txDelay: " << txDelay << std::endl;

	udpSinkFEC->m_udpSent.store(false);

	if (nbBlocksFEC == 0)
	{
	    for (int i = 0; i < UDPSINKFEC_NBORIGINALBLOCKS; i++)
	    {
	        udpSinkFEC->m_socket.SendDataGram((const void *) &txBlockx[i], (int) udpSinkFEC->m_udpSize, udpSinkFEC->m_address, udpSinkFEC->m_port);
	        usleep(txDelay);
	    }
	}
	else
	{
		int originalCount = UDPSINKFEC_NBORIGINALBLOCKS;
		int blockBytes = sizeof(ProtectedBlock);

	    // Fill pointers to data
	    for (int i = 0; i < originalCount + nbBlocksFEC; ++i)
	    {
            txBlockx[i].header.frameIndex = frameIndex;
            txBlockx[i].header.blockIndex = i;

            if (i < originalCount)
            {
            	txDataBlocks[i] = (unsigned char *) &txBlockx[i].protectedBlock;
            }
            else
            {
            	txFecBlocks[i - originalCount] = (unsigned char *) &fecBlocks[i - originalCount];
            }
	    }

	    // Encode FEC blocks
	    FEClib::fec_encode(blockBytes,
	            txDataBlocks,
				originalCount,
				txFecBlocks,
				nbBlocksFEC);

	    // Merge FEC with data to transmit
	    for (int i = 0; i < nbBlocksFEC; i++)
	    {
	        txBlockx[i + originalCount].protectedBlock = fecBlocks[i];
	    }

	    // Transmit all blocks
	    for (int i = 0; i < originalCount + nbBlocksFEC; i++)
	    {
//            std::cerr << "UDPSinkFEC::transmitUDP:"
//                  << " i: " << i
//                  << " frameIndex: " << (int) m_txBlocks[i].header.frameIndex
//                  << " blockIndex: " << (int) m_txBlocks[i].header.blockIndex
//                  << " i.q:";
//
//            for (int j = 0; j < 10; j++)
//            {
//                std::cerr << " " << (int) m_txBlocks[i].protectedBlock.m_samples[j].m_real
//                        << "." << (int) m_txBlocks[i].protectedBlock.m_samples[j].m_imag;
//            }
//
//            std::cerr << std::endl;

	        udpSinkFEC->m_socket.SendDataGram((const void *) &txBlockx[i], (int) udpSinkFEC->m_udpSize, udpSinkFEC->m_address, udpSinkFEC->m_port);
            usleep(txDelay);
	    }
	}

	udpSinkFEC->m_udpSent.store(true);
}
