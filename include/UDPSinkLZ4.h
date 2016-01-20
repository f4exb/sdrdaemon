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

#include <lz4.h>
#include "UDPSink.h"

class UDPSinkLZ4 : public UDPSink
{
public:
	UDPSinkLZ4(const std::string& address, unsigned int port, unsigned int udpSize, unsigned int lz4MinInputSize);
	virtual ~UDPSinkLZ4();
	virtual void write(const IQSampleVector& samples_in);

private:
    uint32_t      m_lz4MinInputSize;    //!< Minimum size of the input data in blocks for compression
    uint32_t      m_lz4MaxInputBlocks;  //!< Maximum number of input hardware blocks to read in one frame
    uint32_t      m_lz4MaxInputSize;    //!< Maximum input size in bytes for the compression of one frame
    uint32_t      m_lz4InputBlockCount; //!< Current number of blocks processed
    uint32_t      m_lz4BufSize;         //!< Size of the LZ4 output buffer
    uint32_t      m_lz4Count;           //!< Current write index in the LZ4 output buffer
    MetaData      m_lz4Meta;            //!< Meta data block specialized for LZ4
    uint8_t*      m_lz4Buffer;          //!< LZ4 output buffer
    LZ4_stream_t* m_lz4Stream;          //!< LZ4 stream control structure

    void udpSend();
    void printMeta(MetaData *metaData);
    void setLZ4Values(uint32_t nbSamples, uint8_t sampleBytes);
};

#endif /* INCLUDE_UDPSINKLZ4_H_ */
