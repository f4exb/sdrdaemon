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
#include "MovingAverage.h"

#define SDRDAEMONFEC_UDPSIZE 512            // UDP payload size
#define SDRDAEMONFEC_NBORIGINALBLOCKS 128   // number of sample blocks per frame excluding FEC blocks
#define SDRDAEMONFEC_NBDECODERSLOTS 4       // power of two sub multiple of int16_t size. A too large one is superfluous.

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
            m_nbFECBlocks = -1;
        }
	};

    struct Sample
    {
        int16_t i;
        int16_t q;
    };

    struct Header
    {
        uint16_t frameIndex;
        uint8_t  blockIndex;
        uint8_t  filler;
    };

    static const int samplesPerBlock = (SDRDAEMONFEC_UDPSIZE - sizeof(Header)) / sizeof(Sample);

    struct ProtectedBlock
    {
        Sample samples[samplesPerBlock];
    };

    struct SuperBlock
    {
        Header         header;
        ProtectedBlock protectedBlock;
    };

    struct ProtectedBlockZero
    {
        MetaDataFEC m_metaData;
        uint8_t     m_filler[SDRDAEMONFEC_UDPSIZE - sizeof(Header) - sizeof(MetaDataFEC)]; // complete for a 512 byte block
    };

#pragma pack(pop)

	SDRdaemonFECBuffer();
	~SDRdaemonFECBuffer();

	/**
	 * Write a superblock to buffer and read a complete data block
	 * \param  array      pointer the input superblock
	 * \param  length     length of superblock
	 * \param  data       pointer to the output data block
	 * \param  dataLength reference to the output data length. This length is 0
	 * \return true if an output data block is available else false
	 */
	bool writeAndRead(uint8_t *array, uint8_t *data, std::size_t& dataLength);
	const MetaDataFEC& getCurrentMeta() const { return m_currentMeta; }
    const MetaDataFEC& getOutputMeta() const { return m_outputMeta; }
	int getCurNbBlocks() const { return m_curNbBlocks; }
	int getCurNbRecovery() const { return m_curNbRecovery; }
	float getAvgNbBlocks() const { return m_avgNbBlocks; }
	float getAvgNbRecovery() const { return m_avgNbRecovery; }

private:
	static const int udpSize = SDRDAEMONFEC_UDPSIZE;
	static const int nbOriginalBlocks = SDRDAEMONFEC_NBORIGINALBLOCKS;
	static const int nbDecoderSlots = SDRDAEMONFEC_NBDECODERSLOTS;

#pragma pack(push, 1)
	struct BufferFrame0
	{
	    ProtectedBlock  m_blocks[nbOriginalBlocks];
	};
    struct BufferFrame
    {
        ProtectedBlock  m_blocks[nbOriginalBlocks - 1];
    };
#pragma pack(pop)

	struct DecoderSlot
    {
        BufferFrame0         m_frame; //!< retrieved frames including block0 with meta data
        ProtectedBlock*      m_originalBlockPtrs[nbOriginalBlocks];
        ProtectedBlock       m_recoveryBlocks[nbOriginalBlocks]; // max size
        CM256::cm256_block   m_cm256DescriptorBlocks[nbOriginalBlocks];
        int                  m_blockCount; //!< total number of blocks received for this frame
        int                  m_recoveryCount; //!< number of recovery blocks received
        bool                 m_decoded; //!< true if decoded
        bool                 m_metaRetrieved;
    };

    void getSlotData(uint8_t *data, std::size_t& dataLength);
    void printMeta(MetaDataFEC *metaData);
    void initDecodeSlot();

	MetaDataFEC          m_currentMeta;  //!< Stored current meta data from input
	MetaDataFEC          m_outputMeta;   //!< Meta data corresponding to output frame
	CM256::cm256_encoder_params m_paramsCM256;
	DecoderSlot          m_decoderSlot;
    //BufferFrame          m_frames[nbDecoderSlots]; in the most general case you would use it as the samples buffer
	int                  m_decoderIndexHead;
	int                  m_frameHead;
	int                  m_curNbBlocks;          //!< (stats) instantaneous number of blocks received
	int                  m_curNbRecovery;        //!< (stats) instantaneous number of recovery blocks used
	MovingAverage<int, int, 10> m_avgNbBlocks;   //!< (stats) average number of blocks received
	MovingAverage<int, int, 10> m_avgNbRecovery; //!< (stats) average number of recovery blocks used
	CM256                m_cm256;
	bool                 m_cm256_OK;
};

#endif /* GR_SDRDAEMONFEC_LIB_SDRDAEMONFECBUFFER_H_ */
