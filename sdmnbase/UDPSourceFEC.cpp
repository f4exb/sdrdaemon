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
    m_socket.BindLocalAddressAndPort(m_address, m_port);
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
    uint8_t data[128*512];
    std::size_t dataLength;

    while (!dataAvailable)
    {
        int received = receiveUDP(this, &superBlock);

        if (received == sizeof(SuperBlock))
        {
            dataAvailable = m_sdmnFECBuffer.writeAndRead((uint8_t*) &superBlock, data, dataLength);
        }
    }

    // Each complete read returns a complete frame of 127*127 samples (127 data blocks of 128 samples less the 1 sample header)
    // that is 127*127*4 = 64516 bytes

    if (dataLength > 0)
    {
        samples_out.resize(dataLength/4);
        memcpy(&samples_out[0], data, dataLength);
        //fprintf(stderr, "UDPSourceFEC::read %lu bytes\n", dataLength); // always 64516 bytes
    }
}

int UDPSourceFEC::receiveUDP(UDPSourceFEC *udpSourceFEC, SuperBlock *superBlock)
{
    std::string fromAddress;
    unsigned short fromPort;
    //fprintf(stderr, "UDPSourceFEC::receiveUDP at %s:%u\n", udpSourceFEC->m_address.c_str(), udpSourceFEC->m_port);
    int nbRead = udpSourceFEC->m_socket.RecvDataGram((void *) superBlock, (int) udpSourceFEC->m_udpSize, fromAddress, fromPort);
    //fprintf(stderr, "UDPSourceFEC::receiveUDP: received %d bytes from %s:%u\n", nbRead, fromAddress.c_str(), fromPort);
    return nbRead;
}
