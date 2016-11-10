///////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2016 F4EXB                                                      //
// written by Edouard Griffiths                                                  //
//                                                                               //
// Integer half-band FIR based interpolator and decimator                        //
// This is the even/odd double buffer variant. Really useful only when SIMD is   //
// used                                                                          //
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

#ifndef INCLUDE_INTHALFBANDFILTER_EO1_H_
#define INCLUDE_INTHALFBANDFILTER_EO1_H_

#include <stdint.h>
#include "HBFilterTraits.h"
#include "IntHalfbandFilterEO1i.h"

template<uint32_t HBFilterOrder>
class IntHalfbandFilterEO1 {
public:
    IntHalfbandFilterEO1();

    void myDecimate(int32_t x1, int32_t y1, int32_t *x2, int32_t *y2)
    {
        storeSample(x1, y1);
        advancePointer();

        storeSample(*x2, *y2);
        doFIR(x2, y2);
        advancePointer();
    }

protected:
    int32_t m_even[2][HBFIRFilterTraits<HBFilterOrder>::hbOrder]; // double buffer technique
    int32_t m_odd[2][HBFIRFilterTraits<HBFilterOrder>::hbOrder]; // double buffer technique

    int m_ptr;
    int m_size;
    int m_state;

    void storeSample(int32_t x, int32_t y)
    {
        if ((m_ptr % 2) == 0)
        {
            m_even[0][m_ptr/2] = x;
            m_even[1][m_ptr/2] = y;
            m_even[0][m_ptr/2 + m_size] = x;
            m_even[1][m_ptr/2 + m_size] = y;
        }
        else
        {
            m_odd[0][m_ptr/2] = x;
            m_odd[1][m_ptr/2] = y;
            m_odd[0][m_ptr/2 + m_size] = x;
            m_odd[1][m_ptr/2 + m_size] = y;
        }
    }

    void advancePointer()
    {
        m_ptr = (m_ptr + 1) % (2*m_size);
    }


    void doFIR(int32_t *x, int32_t *y)
    {
        int32_t iAcc = 0;
        int32_t qAcc = 0;

#ifdef USE_SSE4_1
        IntHalfbandFilterEO1Intrisics<HBFilterOrder>::work(
                m_ptr,
                m_even,
                m_odd,
                iAcc,
                qAcc
        );
#else
        int a = m_ptr/2 + m_size; // tip pointer
        int b = m_ptr/2 + 1; // tail pointer

        for (int i = 0; i < HBFIRFilterTraits<HBFilterOrder>::hbOrder / 4; i++)
        {
            if ((m_ptr % 2) == 0)
            {
                iAcc += (m_even[0][a] + m_even[0][b]) * HBFIRFilterTraits<HBFilterOrder>::hbCoeffs[i];
                qAcc += (m_even[1][a] + m_even[1][b]) * HBFIRFilterTraits<HBFilterOrder>::hbCoeffs[i];
            }
            else
            {
                iAcc += (m_odd[0][a] + m_odd[0][b]) * HBFIRFilterTraits<HBFilterOrder>::hbCoeffs[i];
                qAcc += (m_odd[1][a] + m_odd[1][b]) * HBFIRFilterTraits<HBFilterOrder>::hbCoeffs[i];
            }

            a -= 1;
            b += 1;
        }
#endif
        if ((m_ptr % 2) == 0)
        {
            iAcc += ((int32_t)m_odd[0][m_ptr/2 + m_size/2]) << (HBFIRFilterTraits<HBFilterOrder>::hbShift - 1);
            qAcc += ((int32_t)m_odd[1][m_ptr/2 + m_size/2]) << (HBFIRFilterTraits<HBFilterOrder>::hbShift - 1);
        }
        else
        {
            iAcc += ((int32_t)m_even[0][m_ptr/2 + m_size/2 + 1]) << (HBFIRFilterTraits<HBFilterOrder>::hbShift - 1);
            qAcc += ((int32_t)m_even[1][m_ptr/2 + m_size/2 + 1]) << (HBFIRFilterTraits<HBFilterOrder>::hbShift - 1);
        }

        *x = iAcc >> (HBFIRFilterTraits<HBFilterOrder>::hbShift -1); // HB_SHIFT incorrect do not loose the gained bit
        *y = qAcc >> (HBFIRFilterTraits<HBFilterOrder>::hbShift -1);
    }
};

template<uint32_t HBFilterOrder>
IntHalfbandFilterEO1<HBFilterOrder>::IntHalfbandFilterEO1()
{
    m_size = HBFIRFilterTraits<HBFilterOrder>::hbOrder/2;

    for (int i = 0; i < 2*m_size; i++)
    {
        m_even[0][i] = 0;
        m_even[1][i] = 0;
        m_odd[0][i] = 0;
        m_odd[1][i] = 0;
    }

    m_ptr = 0;
    m_state = 0;
}

#endif /* SDRBASE_DSP_INTHALFBANDFILTEREO_H_ */
