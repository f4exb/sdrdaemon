///////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2016 F4EXB                                                      //
// written by Edouard Griffiths                                                  //
//                                                                               //
// Integer half-band FIR based interpolator and decimator                        //
// This is the even/odd and I/Q stride with double buffering variant             //
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

#ifndef INCLUDE_INTHALFBANDFILTER_ST_H
#define INCLUDE_INTHALFBANDFILTER_ST_H

#include <stdint.h>
#include "HBFilterTraits.h"
#include "IntHalfbandFilterSTi.h"

template<uint32_t HBFilterOrder>
class IntHalfbandFilterST {
public:
    IntHalfbandFilterST();

    void myDecimate(int32_t x1, int32_t y1, int32_t *x2, int32_t *y2)
    {
        storeSample(x1, y1);
        advancePointer();

        storeSample(*x2, *y2);
        doFIR(x2, y2);
        advancePointer();
    }

protected:
    int32_t m_samplesDB[2*HBFilterOrder][2]; // double buffer technique with even/odd amnd I/Q stride
    int32_t m_samplesAligned[HBFilterOrder][2] __attribute__ ((aligned (16)));
	int m_ptr;
	int m_size;
	int m_state;
	int32_t m_iEvenAcc;
	int32_t m_qEvenAcc;
	int32_t m_iOddAcc;
	int32_t m_qOddAcc;


    void storeSample(int32_t x, int32_t y)
    {
        m_samplesDB[m_ptr][0] = x;
        m_samplesDB[m_ptr][1] = y;
        m_samplesDB[m_ptr + m_size][0] = x;
        m_samplesDB[m_ptr + m_size][1] = y;
    }

    void advancePointer()
    {
        m_ptr = m_ptr + 1 < m_size ? m_ptr + 1: 0;
    }

    void doFIR(int32_t *x, int32_t *y)
    {
        // calculate on odd values

        if ((m_ptr % 2) == 1)
        {
            m_iEvenAcc = 0;
            m_qEvenAcc = 0;
            m_iOddAcc = 0;
            m_qOddAcc = 0;

#ifdef USE_SSE4_1
//            memcpy((void *) m_samplesAligned, (const void *) &(m_samplesDB[ m_ptr + 1][0]), HBFilterOrder*2*sizeof(qint32));
            IntHalfbandFilterSTIntrinsics<HBFilterOrder>::workNA(
                    m_ptr + 1,
                    m_samplesDB,
					m_iEvenAcc,
					m_qEvenAcc,
					m_iOddAcc,
					m_qOddAcc);
#else
            int a = m_ptr + m_size; // tip pointer - odd
            int b = m_ptr + 1; // tail pointer - aven

            for (int i = 0; i < HBFIRFilterTraits<HBFilterOrder>::hbOrder / 4; i++)
            {
                m_iEvenAcc += (m_samplesDB[a-1][0] + m_samplesDB[b][0])   * HBFIRFilterTraits<HBFilterOrder>::hbCoeffs[i];
                m_iOddAcc  += (m_samplesDB[a][0]   + m_samplesDB[b+1][0]) * HBFIRFilterTraits<HBFilterOrder>::hbCoeffs[i];
                m_qEvenAcc += (m_samplesDB[a-1][1] + m_samplesDB[b][1])   * HBFIRFilterTraits<HBFilterOrder>::hbCoeffs[i];
                m_qOddAcc  += (m_samplesDB[a][1]   + m_samplesDB[b+1][1]) * HBFIRFilterTraits<HBFilterOrder>::hbCoeffs[i];
                a -= 2;
                b += 2;
            }
#endif
            m_iEvenAcc += ((int32_t)m_samplesDB[m_ptr + m_size/2][0]) << (HBFIRFilterTraits<HBFilterOrder>::hbShift - 1);
            m_qEvenAcc += ((int32_t)m_samplesDB[m_ptr + m_size/2][1]) << (HBFIRFilterTraits<HBFilterOrder>::hbShift - 1);
            m_iOddAcc += ((int32_t)m_samplesDB[m_ptr + m_size/2 + 1][0]) << (HBFIRFilterTraits<HBFilterOrder>::hbShift - 1);
            m_qOddAcc += ((int32_t)m_samplesDB[m_ptr + m_size/2 + 1][1]) << (HBFIRFilterTraits<HBFilterOrder>::hbShift - 1);

            *x = m_iEvenAcc >> HBFIRFilterTraits<HBFilterOrder>::hbShift -1;
            *y = m_qEvenAcc >> HBFIRFilterTraits<HBFilterOrder>::hbShift -1;
        }
        else
        {
            *x = m_iOddAcc >> HBFIRFilterTraits<HBFilterOrder>::hbShift -1;
            *y = m_qOddAcc >> HBFIRFilterTraits<HBFilterOrder>::hbShift -1;
        }
    }
};

template<uint32_t HBFilterOrder>
IntHalfbandFilterST<HBFilterOrder>::IntHalfbandFilterST()
{
    m_size = HBFIRFilterTraits<HBFilterOrder>::hbOrder;

    for (int i = 0; i < m_size; i++)
    {
        m_samplesDB[i][0] = 0;
        m_samplesDB[i][1] = 0;
    }

    m_ptr = 0;
    m_state = 0;
    m_iEvenAcc = 0;
    m_qEvenAcc = 0;
    m_iOddAcc = 0;
    m_qOddAcc = 0;
}

#endif // INCLUDE_INTHALFBANDFILTER_DB_H
