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

#ifndef INCLUDE_INTHALFBANDFILTER_EO2_H_
#define INCLUDE_INTHALFBANDFILTER_EO2_H_

#include <stdint.h>

#ifdef USE_SSE4_1
#include <smmintrin.h>
#endif

#ifdef USE_NEON
#include <arm_neon.h>
#endif

#include "HBFilterTraits.h"
#include "IntHalfbandFilterEO2i.h"

template<uint32_t HBFilterOrder>
class IntHalfbandFilterEO2 {
public:
    IntHalfbandFilterEO2();

    void myDecimate(int32_t x1, int32_t y1, int32_t *x2, int32_t *y2)
    {
        storeSample(x1, y1);
        advancePointer();

        storeSample(*x2, *y2);
        doFIR(x2, y2);
        advancePointer();
    }

protected:
    int32_t m_evenB[2][HBFIRFilterTraits<HBFilterOrder>::hbOrder]; // double buffer technique
    int32_t m_oddB[2][HBFIRFilterTraits<HBFilterOrder>::hbOrder]; // double buffer technique
    int32_t m_evenA[2][HBFIRFilterTraits<HBFilterOrder>::hbOrder]; // double buffer technique
    int32_t m_oddA[2][HBFIRFilterTraits<HBFilterOrder>::hbOrder]; // double buffer technique

    int m_ptrA;
    int m_ptrB;
    int m_size;
    int m_state;

    void storeSample(int32_t x, int32_t y)
    {
        if ((m_ptrB % 2) == 0)
        {
            m_evenB[0][m_ptrB/2] = x;
            m_evenB[1][m_ptrB/2] = y;
            m_evenB[0][m_ptrB/2 + m_size] = x;
            m_evenB[1][m_ptrB/2 + m_size] = y;
            m_evenA[0][m_ptrA/2] = x;
            m_evenA[1][m_ptrA/2] = y;
            m_evenA[0][m_ptrA/2 + m_size] = x;
            m_evenA[1][m_ptrA/2 + m_size] = y;
        }
        else
        {
            m_oddB[0][m_ptrB/2] = x;
            m_oddB[1][m_ptrB/2] = y;
            m_oddB[0][m_ptrB/2 + m_size] = x;
            m_oddB[1][m_ptrB/2 + m_size] = y;
            m_oddA[0][m_ptrA/2] = x;
            m_oddA[1][m_ptrA/2] = y;
            m_oddA[0][m_ptrA/2 + m_size] = x;
            m_oddA[1][m_ptrA/2 + m_size] = y;
        }
    }

    void advancePointer()
    {
        m_ptrA = (m_ptrA - 1 + 2*m_size) % (2*m_size);
        m_ptrB = (m_ptrB + 1) % (2*m_size);
    }

    void doFIR(int32_t *x, int32_t *y)
    {
        int32_t iAcc = 0;
        int32_t qAcc = 0;

#if defined(USE_SSE4_1) || defined(USE_NEON)
        IntHalfbandFilterEO2Intrisics<HBFilterOrder>::work(
                m_ptrA,
                m_ptrB,
                m_evenA,
                m_evenB,
                m_oddA,
                m_oddB,
                iAcc,
                qAcc
        );
#else
        int a = ptrA/2; // tip pointer
        int b = ptrB/2 + 1; // tail pointer

        for (int i = 0; i < HBFIRFilterTraits<HBFilterOrder>::hbOrder / 4; i++)
        {
            if ((m_ptrB % 2) == 0)
            {
                iAcc += (m_evenA[0][a] + m_evenB[0][b]) * HBFIRFilterTraits<HBFilterOrder>::hbCoeffs[i];
                qAcc += (m_evenA[1][a] + m_evenB[1][b]) * HBFIRFilterTraits<HBFilterOrder>::hbCoeffs[i];
            }
            else
            {
                iAcc += (m_oddA[0][a] + m_oddB[0][b]) * HBFIRFilterTraits<HBFilterOrder>::hbCoeffs[i];
                qAcc += (m_oddA[1][a] + m_oddB[1][b]) * HBFIRFilterTraits<HBFilterOrder>::hbCoeffs[i];
            }

            a += 1;
            b += 1;
        }
#endif

        if ((m_ptrB % 2) == 0)
        {
            iAcc += ((int32_t)m_oddB[0][m_ptrB/2 + m_size/2]) << (HBFIRFilterTraits<HBFilterOrder>::hbShift - 1);
            qAcc += ((int32_t)m_oddB[1][m_ptrB/2 + m_size/2]) << (HBFIRFilterTraits<HBFilterOrder>::hbShift - 1);
        }
        else
        {
            iAcc += ((int32_t)m_evenB[0][m_ptrB/2 + m_size/2 + 1]) << (HBFIRFilterTraits<HBFilterOrder>::hbShift - 1);
            qAcc += ((int32_t)m_evenB[1][m_ptrB/2 + m_size/2 + 1]) << (HBFIRFilterTraits<HBFilterOrder>::hbShift - 1);
        }

        *x = iAcc >> (HBFIRFilterTraits<HBFilterOrder>::hbShift -1); // HB_SHIFT incorrect do not loose the gained bit
        *y = qAcc >> (HBFIRFilterTraits<HBFilterOrder>::hbShift -1);
    }
};

template<uint32_t HBFilterOrder>
IntHalfbandFilterEO2<HBFilterOrder>::IntHalfbandFilterEO2()
{
    m_size = HBFIRFilterTraits<HBFilterOrder>::hbOrder/2;

    for (int i = 0; i < 2*m_size; i++)
    {
        m_evenB[0][i] = 0;
        m_evenB[1][i] = 0;
        m_oddB[0][i] = 0;
        m_oddB[1][i] = 0;
    }

    m_ptrA = 0;
    m_ptrB = 0;
    m_state = 0;
}

#endif /* INCLUDE_INTHALFBANDFILTER_EO2_H_ */
