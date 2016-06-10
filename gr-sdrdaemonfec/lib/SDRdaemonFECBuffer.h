///////////////////////////////////////////////////////////////////////////////////
// SDRdaemon - send I/Q samples read from a SDR device over the network via UDP  //
//             with FEC protection. GNUradio interface.                          //
//                                                                               //
// This is an adaptation of the GNUradio UDP source                              //
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

#ifndef GR_SDRDAEMONFEC_LIB_SDRDAEMONFECBUFFER_H_
#define GR_SDRDAEMONFEC_LIB_SDRDAEMONFECBUFFER_H_

#include <stdint.h>
#include <cstddef>
#include "cm256.h"

#define SDRDAEMONFEC_UDPSIZE 512
#define SDRDAEMONFEC_NBORIGINALBLOCKS 128
#define SDRDAEMONFEC_NBDECODERSLOTS 4

class SDRdaemonFECBuffer
{
public:
#pragma pack(push, 1)
	struct MetaDataFEC
	{
        uint32_t m_centerFrequency;   //!<  4 center frequency in kHz
        uint32_t m_sampleRate;        //!<  8 sample rate in Hz
        uint8_t  m_sampleBytes;       //!<  9 MSB(4): indicators, LSB(4) number of bytes per sample
        uint8_t  m_sampleBits;        //!< 10 number of effective bits per sample
        uint8_t  m_nbOriginalBlocks;  //!< 11 number of blocks with original (protected) data
        uint8_t  m_nbFECBlocks;       //!< 12 number of blocks carrying FEC
        uint32_t m_tv_sec;            //!< 16 seconds of timestamp at start time of super-frame processing
        uint32_t m_tv_usec;           //!< 20 microseconds of timestamp at start time of super-frame processing

        bool operator==(const MetaDataFEC& rhs)
        {
            return (memcmp((const void *) this, (const void *) &rhs, 12) == 0); // Only the 12 first bytes are relevant
        }

        void init()
        {
            memset((void *) this, 0, sizeof(MetaDataFEC));
        }
	};

    struct Header
    {
        uint16_t frameIndex;
        uint8_t  blockIndex;
        uint8_t  filler;
    };

    struct Sample
    {
        uint16_t i;
        uint16_t q;
    };

    static const int samplesPerBlock = (SDRDAEMONFEC_UDPSIZE - sizeof(Header)) / sizeof(Sample);

    struct ProtectedBlock
    {
        Sample samples[samplesPerBlock];
    };
    struct SuperBlock
    {
        Header         header;
        Sample         samples[samplesPerBlock];
        ProtectedBlock protectedBlock;
    };
#pragma pack(pop)

	SDRdaemonFECBuffer();
	~SDRdaemonFECBuffer();

	/**
	 * @return: true if data is available
	 */
	bool writeAndRead(uint8_t *array, std::size_t length, uint8_t *data, std::size_t& dataLength);

	const MetaDataFEC& getCurrentMeta() const { return m_currentMeta; }

private:
	static const int samplesPerBlockZero = samplesPerBlock - (sizeof(MetaDataFEC) / sizeof(Sample));
	static const int udpSize = SDRDAEMONFEC_UDPSIZE;
	static const int nbOriginalBlocks = SDRDAEMONFEC_NBORIGINALBLOCKS;
	static const int nbDecoderSlots = SDRDAEMONFEC_NBDECODERSLOTS;

#pragma pack(push, 1)
	struct BlockZero
	{
	    MetaDataFEC m_metaData;
	    Sample      m_samples[samplesPerBlockZero];
	};

	struct DecoderSlot
	{
	    BlockZero      m_blockZero;
	    Sample*        m_samplesPtrs[nbOriginalBlocks - 1];
	    ProtectedBlock recoveryBuffer[nbOriginalBlocks]; // max size
	};
#pragma pack(pop)

    void printMeta(MetaDataFEC *metaData);

	MetaDataFEC          m_currentMeta;  //!< Stored current meta data
	cm256_encoder_params m_paramsCM256;
	DecoderSlot          m_decoderSlots[nbDecoderSlots];
	int                  m_decoderSlotHead;
	int                  m_frameHead;
};

#endif /* GR_SDRDAEMONFEC_LIB_SDRDAEMONFECBUFFER_H_ */
