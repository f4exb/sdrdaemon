///////////////////////////////////////////////////////////////////////////////////
// SDRdaemon - send I/Q samples read from a SDR device over the network via UDP. //
//                                                                               //
// Copyright (C) 2015 Edouard Griffiths, F4EXB                                   //
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

#ifndef INCLUDE_UDPSINKLZ4_H_
#define INCLUDE_UDPSINKLZ4_H_

#include <atomic>
#include <lz4.h>
#include "UDPSink.h"

class UDPSinkLZ4 : public UDPSink
{
public:
	UDPSinkLZ4(const std::string& address, unsigned int port, unsigned int udpSize, unsigned int minFrameSize);
	virtual ~UDPSinkLZ4();
	uint32_t compressInput();
	virtual void write(const IQSampleVector& samples_in);
    virtual void setTxDelay(int txDelay);

private:
    void udpSend();
    void updateSizes(MetaData *metaData);
    void printMeta(MetaData *metaData);

    MetaData m_currentMeta;     //!< meta data according to current values
    MetaData m_sendMeta;        //!< meta data to send over UDP
    uint32_t m_minFrameSize;    //!< minimal size of data frame in bytes
    uint32_t m_hardBlockSize;   //!< size of a hardware block in bytes (obtained from the device)
    uint32_t m_maxInputBlocks;  //!< maximum number of input blocks to satisfy the minimum frame size
    uint32_t m_inputBlockCount; //!< current count of input blocks in the input frame
    uint8_t  *m_inputBuffer;    //!< input data frame buffer
    uint32_t m_maxOutputSize;   //!< Maximum compressed output size
    uint8_t  *m_outputBuffer;   //!< Output compressed data buffer

    std::atomic_int m_txDelay;  //!< Delay in microseconds (usleep) between each sending of an UDP datagram
};

#endif /* INCLUDE_UDPSINKLZ4_H_ */
