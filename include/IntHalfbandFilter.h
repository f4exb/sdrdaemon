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

#ifndef INCLUDE_INTHALFBANDFILTER_H_
#define INCLUDE_INTHALFBANDFILTER_H_

#include <cstdint>

/*
 * supported filter orders: 80, 64, 48, 32
 * any usage of another value will be prevented by compilation errors
 */
template<uint32_t HBFilterOrder>
struct HBFIRFilterTraits
{
};

template<>
struct HBFIRFilterTraits<32>
{
    static const uint32_t hbOrder = 32;
    static const uint32_t hbShift = 14;
    static const uint32_t hbMod[32+6];
    static const int32_t  hbCoeffs[8];
};

template<>
struct HBFIRFilterTraits<48>
{
    static const uint32_t hbOrder = 48;
    static const uint32_t hbShift = 14;
    static const uint32_t hbMod[48+6];
    static const int32_t  hbCoeffs[12];
};

template<>
struct HBFIRFilterTraits<64>
{
    static const uint32_t hbOrder = 64;
    static const uint32_t hbShift = 14;
    static const uint32_t hbMod[64+6];
    static const int32_t  hbCoeffs[16];
};

template<>
struct HBFIRFilterTraits<80>
{
    static const uint32_t hbOrder = 80;
    static const uint32_t hbShift = 14;
    static const uint32_t hbMod[80+6];
    static const int32_t  hbCoeffs[20];
};

/** Slimmed out class from SDRangel's IntHalfbandFilter */
template<uint32_t HBFilterOrder>
class IntHalfbandFilter
{
public:
	IntHalfbandFilter();

	void myDecimate(int32_t x1, int32_t y1, int32_t *x2, int32_t *y2)
	{
		m_samples[m_ptr][0] = x1;
		m_samples[m_ptr][1] = y1;
		m_ptr = HBFIRFilterTraits<HBFilterOrder>::hbMod[m_ptr + 2 - 1];

		m_samples[m_ptr][0] = *x2;
		m_samples[m_ptr][1] = *y2;

		doFIR(x2, y2);

		m_ptr = HBFIRFilterTraits<HBFilterOrder>::hbMod[m_ptr + 2 - 1];
	}

protected:
	int32_t m_samples[HBFIRFilterTraits<HBFilterOrder>::hbOrder + 1][2];
	int16_t m_ptr;
	int m_state;

	void doFIR(int32_t *x, int32_t *y)
	{
		// init read-pointer
		int a = HBFIRFilterTraits<HBFilterOrder>::hbMod[m_ptr + 2 + 1]; // 0 + 1
		int b = HBFIRFilterTraits<HBFilterOrder>::hbMod[m_ptr + 2 - 2]; //-1 - 1

		// go through samples in buffer
		int32_t iAcc = 0;
		int32_t qAcc = 0;

		for (unsigned int i = 0; i < HBFIRFilterTraits<HBFilterOrder>::hbOrder / 4; i++)
		{
			// do multiply-accumulate
			//qint32 iTmp = m_samples[a][0] + m_samples[b][0]; // Valgrind optim
			//qint32 qTmp = m_samples[a][1] + m_samples[b][1]; // Valgrind optim
			iAcc += (m_samples[a][0] + m_samples[b][0]) * HBFIRFilterTraits<HBFilterOrder>::hbCoeffs[i];
			qAcc += (m_samples[a][1] + m_samples[b][1]) * HBFIRFilterTraits<HBFilterOrder>::hbCoeffs[i];

			// update read-pointer
			a = HBFIRFilterTraits<HBFilterOrder>::hbMod[a + 2 + 2];
			b = HBFIRFilterTraits<HBFilterOrder>::hbMod[b + 2 - 2];
		}

		a = HBFIRFilterTraits<HBFilterOrder>::hbMod[a + 2 - 1];

		iAcc += ((int32_t)m_samples[a][0] + 1) << (HBFIRFilterTraits<HBFilterOrder>::hbShift - 1);
		qAcc += ((int32_t)m_samples[a][1] + 1) << (HBFIRFilterTraits<HBFilterOrder>::hbShift - 1);

		*x = iAcc >> (HBFIRFilterTraits<HBFilterOrder>::hbShift -1); // HB_SHIFT incorrect do not loose the gained bit
		*y = qAcc >> (HBFIRFilterTraits<HBFilterOrder>::hbShift -1);
	}
};

template<uint32_t HBFilterOrder>
IntHalfbandFilter<HBFilterOrder>::IntHalfbandFilter()
{
    for(unsigned int i = 0; i < HBFIRFilterTraits<HBFilterOrder>::hbOrder + 1; i++)
    {
        m_samples[i][0] = 0;
        m_samples[i][1] = 0;
    }

    m_ptr = 0;
    m_state = 0;
}

#endif /* INCLUDE_INTHALFBANDFILTER_H_ */
