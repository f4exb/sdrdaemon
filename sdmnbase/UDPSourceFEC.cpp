///////////////////////////////////////////////////////////////////////////////////
// SDRdaemon - receive I/Q samples over the network via UDP and write to a       //
//             SDR device .                                                      //
//                                                                               //
// Copyright (C) 2017 Edouard Griffiths, F4EXB                                   //
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
#include <boost/crc.hpp>
#include <boost/cstdint.hpp>
#include "UDPSourceFEC.h"

//#define SDRDAEMON_PUNCTURE 101 // debug: test FEC

UDPSourceFEC::UDPSourceFEC(const std::string& address, unsigned int port) :
    UDPSource::UDPSource(address, port, UDPSOURCEFEC_UDPSIZE),
    m_nbBlocksFEC(0),
    m_rxThread(0),
	m_rxBlockIndex(0),
	m_rxBlocksIndex(0),
	m_frameCount(0),
	m_sampleIndex(0)
{
    m_currentMetaFEC.init();
    m_udpReceived.store(true);
}

UDPSourceFEC::~UDPSourceFEC()
{
	if (m_rxThread)
	{
		m_rxThread->join();
		delete m_rxThread;
	}
}

void UDPSourceFEC::read(IQSampleVector& samples_out)
{
    SuperBlock superBlock;
    bool dataAvailable = false;
    uint8_t *data = 0;
    std::size_t dataLength;

    while (!dataAvailable)
    {
        int received = receiveUDP(this, &superBlock);

        if (received == sizeof(SuperBlock))
        {
            dataAvailable = m_sdmnFECBuffer.writeAndRead((uint8_t*) &superBlock, data, dataLength);
        }
    }

    if (data)
    {
        samples_out.resize(dataLength/4);
        memcpy(&samples_out[0], data, dataLength);
    }
}

int UDPSourceFEC::receiveUDP(UDPSourceFEC *udpSourceFEC, SuperBlock *superBlock)
{
    return udpSourceFEC->m_socket.RecvDataGram((void *) superBlock, (int) udpSourceFEC->m_udpSize, udpSourceFEC->m_address, udpSourceFEC->m_port);
}
