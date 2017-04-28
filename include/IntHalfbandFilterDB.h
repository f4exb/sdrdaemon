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

#ifndef INCLUDE_INTHALFBANDFILTER_DB_H_
#define INCLUDE_INTHALFBANDFILTER_DB_H_

#include <cstdint>
#include "HBFilterTraits.h"

/** Slimmed out class from SDRangel's IntHalfbandFilter */
template<uint32_t HBFilterOrder>
class IntHalfbandFilterDB
{
public:
    IntHalfbandFilterDB();

	void myDecimate(int32_t x1, int32_t y1, int32_t *x2, int32_t *y2)
	{
		m_samples[m_ptr][0] = x1;
		m_samples[m_ptr][1] = y1;
        m_samples[m_ptr + m_size][0] = x1;
        m_samples[m_ptr + m_size][1] = y1;

		m_ptr = (m_ptr + 1) % m_size;

		m_samples[m_ptr][0] = *x2;
		m_samples[m_ptr][1] = *y2;
        m_samples[m_ptr + m_size][0] = *x2;
        m_samples[m_ptr + m_size][1] = *y2;

		doFIR(x2, y2);

        m_ptr = (m_ptr + 1) % m_size;
	}

    void myInterpolate(int32_t *x1, int32_t *y1, int32_t *x2, int32_t *y2)
    {
        // insert sample into ring double buffer
        m_samples[m_ptr][0] = *x1;
        m_samples[m_ptr][1] = *y1;
        m_samples[m_ptr + HBFIRFilterTraits<HBFilterOrder>::hbOrder/2][0] = *x1;
        m_samples[m_ptr + HBFIRFilterTraits<HBFilterOrder>::hbOrder/2][1] = *y1;

        // advance pointer
        if (m_ptr < (HBFIRFilterTraits<HBFilterOrder>::hbOrder/2) - 1) {
            m_ptr++;
        } else {
            m_ptr = 0;
        }

        // first output sample calculated with the middle peak
        *x1 = m_samples[m_ptr + (HBFIRFilterTraits<HBFilterOrder>::hbOrder/4) - 1][0];
        *y1 = m_samples[m_ptr + (HBFIRFilterTraits<HBFilterOrder>::hbOrder/4) - 1][1];

        // second sample calculated with the filter
        doInterpolateFIR(x2, y2);
    }
protected:
	int32_t m_samples[2*(HBFIRFilterTraits<HBFilterOrder>::hbOrder - 1)][2]; // double buffer technique
	int16_t m_ptr;
	int m_size;
	int m_state;

	void doFIR(int32_t *x, int32_t *y)
	{
		// init read-pointer
        int a = m_ptr + m_size; // tip pointer
        int b = m_ptr + 1; // tail pointer

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
            a -= 2;
            b += 2;
		}

		iAcc += ((int32_t)m_samples[b-1][0] + 1) << (HBFIRFilterTraits<HBFilterOrder>::hbShift - 1);
		qAcc += ((int32_t)m_samples[b-1][1] + 1) << (HBFIRFilterTraits<HBFilterOrder>::hbShift - 1);

		*x = iAcc >> (HBFIRFilterTraits<HBFilterOrder>::hbShift -1); // HB_SHIFT incorrect do not loose the gained bit
		*y = qAcc >> (HBFIRFilterTraits<HBFilterOrder>::hbShift -1);
	}

    void doInterpolateFIR(int32_t *x, int32_t *y)
    {
        int a = m_ptr;
        int b = m_ptr + (HBFIRFilterTraits<HBFilterOrder>::hbOrder / 2) - 1;

        // go through samples in buffer
        int32_t iAcc = 0;
        int32_t qAcc = 0;

        for (int i = 0; i < HBFIRFilterTraits<HBFilterOrder>::hbOrder / 4; i++)
        {
            iAcc += (m_samples[a][0] + m_samples[b][0]) * HBFIRFilterTraits<HBFilterOrder>::hbCoeffs[i];
            qAcc += (m_samples[a][1] + m_samples[b][1]) * HBFIRFilterTraits<HBFilterOrder>::hbCoeffs[i];
            a++;
            b--;
        }

        *x = iAcc >> (HBFIRFilterTraits<HBFilterOrder>::hbShift -1);
        *y = qAcc >> (HBFIRFilterTraits<HBFilterOrder>::hbShift -1);
    }
};

template<uint32_t HBFilterOrder>
IntHalfbandFilterDB<HBFilterOrder>::IntHalfbandFilterDB()
{
    m_size = HBFIRFilterTraits<HBFilterOrder>::hbOrder - 1;

    for(int i = 0; i < 2*m_size; i++)
    {
        m_samples[i][0] = 0;
        m_samples[i][1] = 0;
    }

    m_ptr = 0;
    m_state = 0;
}

#endif /* INCLUDE_INTHALFBANDFILTER_H_ */
