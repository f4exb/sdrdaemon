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

#ifndef INCLUDE_UDPSINK_H_
#define INCLUDE_UDPSINK_H_

#include <string.h>

#include "SDRDaemon.h"
#include "UDPSocket.h"
#include "CRC64.h"

/** Downsampler and UDP sink for samples read from the device */
class UDPSink
{
public:
#pragma pack(push, 1)
	struct MetaData
	{
		uint64_t m_centerFrequency;   //!< center frequency in Hz
		uint32_t m_sampleRate;        //!< sample rate in Hz
		uint8_t  m_sampleBytes;       //!< number of bytes per sample + MSB: remainder sent first in meta block
		uint8_t  m_sampleBits;        //!< number of effective bits per sample
		uint16_t m_blockSize;         //!< payload size
		uint32_t m_nbSamples;         //!< total number of samples sent in this frame
		uint16_t m_remainderSamples;  //!< number of remainder I/Q samples
		uint16_t m_nbCompleteBlocks;  //!< number of blocks full of samples
		uint32_t m_tv_sec;            //!< seconds of timestamp at start time of frame processing
		uint32_t m_tv_usec;           //!< microseconds of timestamp at start time of frame processing
		uint64_t m_crc;               //!< 64 bit CRC of the above

		bool operator==(const MetaData& rhs)
		{
		    return (memcmp((const void *) this, (const void *) &rhs, sizeof(MetaData) - 16) == 0);
		}

		void init()
		{
			memset((void *) this, 0, sizeof(MetaData));
		}

		void operator=(const MetaData& rhs)
		{
			memcpy((void *) this, (const void *) &rhs, sizeof(MetaData));
		}
	};
#pragma pack(pop)



	/**
	 * Construct UDP sink
	 *
	 * address          :: Address where the samples are sent
	 * port             :: UDP port where the samples are sent
	 * udpSize          :: Size of UDP block in number of samples
	 */
	UDPSink(const std::string& address, unsigned int port, unsigned int udpSize);

	/** Destroy UDP sink */
	~UDPSink();

    /**
     * Write IQ samples to UDP port.
     */
	void write(const IQSampleVector& samples_in);

    /** Return the last error, or return an empty string if there is no error. */
    std::string error()
    {
        std::string ret(m_error);
        m_error.clear();
        return ret;
    }

    void setCenterFrequency(uint64_t centerFrequency) { m_centerFrequency = centerFrequency; }
    void setSampleRate(uint32_t sampleRate) { m_sampleRate = sampleRate; }
    void setSampleBytes(uint8_t sampleBytes) { m_sampleBytes = sampleBytes; }
    void setSampleBits(uint8_t sampleBits) { m_sampleBits = sampleBits; }

    /** Return true if the stream is OK, return false if there is an error. */
    operator bool() const
    {
        return m_error.empty();
    }

private:
    std::string  m_address; //!< UDP foreign address
	unsigned int m_port;    //!< UDP foreign port
	unsigned int m_udpSize; //!< Size of UDP block in number of samples
    std::string  m_error;

    uint64_t     m_centerFrequency;   //!< center frequency in Hz
    uint32_t     m_sampleRate;        //!< sample rate in Hz
    uint8_t      m_sampleBytes;       //!< number of bytes per sample + MSB: remainder sent first in meta block
    uint8_t      m_sampleBits;        //!< number of effective bits per sample
    uint32_t     m_nbSamples;         //!< total number of samples sent int the last frame

    UDPSocket    m_socket;
    MetaData     m_currentMeta;
    CRC64        m_crc64;
    uint8_t*     m_bufMeta;
    uint8_t*     m_buf;
};



#endif /* INCLUDE_UDPSINK_H_ */
