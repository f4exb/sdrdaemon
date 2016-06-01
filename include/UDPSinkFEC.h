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

#ifndef INCLUDE_UDPSINKFEC_H_
#define INCLUDE_UDPSINKFEC_H_

#include "UDPSink.h"

#define UDPSINKFEC_SUBFRAMESIZE 64
#define UDPSINKFEC_NBSUBFRAMES 6

class UDPSinkFEC : public UDPSink
{
public:
    UDPSinkFEC(const std::string& address, unsigned int port);
    virtual ~UDPSinkFEC();
    virtual void write(const IQSampleVector& samples_in);
    virtual void setNbBlocksFEC(int nbBlocksFEC) { m_nbBlocksFEC = nbBlocksFEC; }

private:
#pragma pack(push, 1)
    struct ProtectedBlock
    {
        uint8_t m_blockIndex;
        uint8_t m_data[UDPSINKFEC_NBSUBFRAMES * UDPSINKFEC_SUBFRAMESIZE];
    };
    struct SuperBlock
    {
        uint16_t       m_frameIndex;
        uint8_t        m_blockIndex;
        ProtectedBlock m_protectedBlock;
    };
#pragma pack(pop)

    int m_nbBlocksFEC;

    static const int m_subFrameSize;
    static const int m_nbSubFrames;
    static const int m_frameSize;
};


#endif /* INCLUDE_UDPSINKFEC_H_ */
