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

/* Data structure:
 *
 * Fi: Frame index (Protected UDP blocks: original+FEC. This frame contains data super-frame)
 * Bi: Block index (UDP block index in protected UDP blocks)
 * sF: Sub-frame: 16 I/Q samples (64 bytes) or meta data (first UDP block in protected UDP blocks)
 * OB: SuperBlock: original block containing a protected block
 * FB: SuperBlock: FEC block
 *
 * One SuperBlock:
 *
 * |Fi|Bi|Bi|sF|sF|sF|sF|sF|sF| ....
 * <--------------------------> SuperBlock
 *       <--------------------> ProtectedBlock or FEC block
 *
 * Complete transmission:
 *
 * |OB|OB|OB|...|OB|FB|...|FB| : 128 OBs and 1 to 128 FBs
 *
 * 128 Original blocks are protected with 1 to 128 redundancy (FEC) blocks. This constitutes a transmission frame
 * A transmission frame transmits a data super-frame carried by original blocks
 * A transmission frame is composed of SuperBlocks that have a frame index. The block index is repeated in the SuperBlock as it is used by the decoder.
 *   It also serves the purpose of ordering original blocks and therefore is repeated inside the protected block
 * A data super-frame is composed of 128 data frames
 * A data frame is transported in a ProtectefBlock
 * A data frame is composed of 6 sub-frames
 * A sub-frame is either a meta data frame (first one of a data super-frame) or 16 I/Q 2x2 bytes samples (64 bytes)
 *
*/

#ifndef INCLUDE_UDPSOURCEFEC_H_
#define INCLUDE_UDPSOURCEFEC_H_

#include <atomic>
#include <vector>
#include <string>
#include "UDPSource.h"
#include "SDRdaemonFECBuffer.h"

#define UDPSOURCEFEC_UDPSIZE 512
#define UDPSOURCEFEC_NBORIGINALBLOCKS 128

namespace std
{
    class thread;
}

class UDPSourceFEC : public UDPSource
{
public:
    UDPSourceFEC(const std::string& address, unsigned int port);
    virtual ~UDPSourceFEC();

    /**
     * Read IQ samples from UDP port. Returns a complete protected frame of 127*127 samples
     */
    virtual void read(IQSampleVector& samples_in);

    /**
     * Format a status message in the given string
     */
    virtual void getStatusMessage(char *messageBuffer);

private:
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
        uint32_t m_crc32;             //!< 24 CRC32 of the above

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

    struct Header
    {
        uint16_t frameIndex;
        uint8_t  blockIndex;
        uint8_t  filler;
    };

    static const int samplesPerBlock = (UDPSOURCEFEC_UDPSIZE - sizeof(Header)) / sizeof(IQSample);

    struct ProtectedBlock
    {
        IQSample m_samples[samplesPerBlock];
    };

    struct SuperBlock
    {
        Header         header;
        ProtectedBlock protectedBlock;
    };
#pragma pack(pop)

    SDRdaemonFECBuffer m_sdmnFECBuffer;  //!< FEC handling buffer
    MetaDataFEC m_currentMetaFEC;        //!< Meta data for current frame
    SuperBlock m_rxBlocks[4][256];       //!< UDP blocks received with original data + FEC
    std::thread *m_rxThread;             //!< Thread to transmit UDP blocks
    SuperBlock m_superBlock;             //!< current super block being built
    //ProtectedBlock m_fecBlocks[256];     //!< FEC data
    int m_rxBlockIndex;                  //!< Current index in blocks to transmit in the Tx row
    int m_rxBlocksIndex;                 //!< Current index of Tx blocks row
    uint16_t m_frameCount;               //!< transmission frame count
    int m_sampleIndex;                   //!< Current sample index in protected block data
    std::atomic_bool m_udpReceived;      //!< True when UDP receiving thread has finished (Frame reception complete)

    static int receiveUDP(UDPSourceFEC *udpSourceFEC, SuperBlock *superBlock);
};


#endif /* INCLUDE_UDPSOURCEFEC_H_ */
