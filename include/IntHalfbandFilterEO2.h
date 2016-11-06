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
        int a = m_ptrA/2; // tip pointer
        int b = m_ptrB/2 + 1; // tail pointer

        int32_t iAcc = 0;
        int32_t qAcc = 0;

#if defined(USE_SSE4_1)
//#warning "IntHalfbandFiler SIMD"
        const __m128i* h = (const __m128i*) HBFIRFilterTraits<HBFilterOrder>::hbCoeffs;
        __m128i sumI = _mm_setzero_si128();
        __m128i sumQ = _mm_setzero_si128();
        __m128i sa, sb;

        for (int i = 0; i < HBFIRFilterTraits<HBFilterOrder>::hbOrder / 16; i++)
        {
            if ((m_ptrB % 2) == 0)
            {
                sa = _mm_loadu_si128((__m128i*) &(m_evenA[0][a]));
                sb = _mm_loadu_si128((__m128i*) &(m_evenB[0][b]));
                sumI = _mm_add_epi32(sumI, _mm_mullo_epi32(_mm_add_epi32(sa, sb), *h));

                sa = _mm_loadu_si128((__m128i*) &(m_evenA[1][a]));
                sb = _mm_loadu_si128((__m128i*) &(m_evenB[1][b]));
                sumQ = _mm_add_epi32(sumQ, _mm_mullo_epi32(_mm_add_epi32(sa, sb), *h));
            }
            else
            {
                sa = _mm_loadu_si128((__m128i*) &(m_oddA[0][a]));
                sb = _mm_loadu_si128((__m128i*) &(m_oddB[0][b]));
                sumI = _mm_add_epi32(sumI, _mm_mullo_epi32(_mm_add_epi32(sa, sb), *h));

                sa = _mm_loadu_si128((__m128i*) &(m_oddA[1][a]));
                sb = _mm_loadu_si128((__m128i*) &(m_oddB[1][b]));
                sumQ = _mm_add_epi32(sumQ, _mm_mullo_epi32(_mm_add_epi32(sa, sb), *h));
            }

            a += 4;
            b += 4;
            ++h;
        }

        // horizontal add of four 32 bit partial sums

        sumI = _mm_add_epi32(sumI, _mm_srli_si128(sumI, 8));
        sumI = _mm_add_epi32(sumI, _mm_srli_si128(sumI, 4));
        iAcc = _mm_cvtsi128_si32(sumI);

        sumQ = _mm_add_epi32(sumQ, _mm_srli_si128(sumQ, 8));
        sumQ = _mm_add_epi32(sumQ, _mm_srli_si128(sumQ, 4));
        qAcc = _mm_cvtsi128_si32(sumQ);
#elif defined(USE_NEON)
        int32x4_t sumI = vdupq_n_s32(0);
        int32x4_t sumQ = vdupq_n_s32(0);
        int32x4_t sa, sb, sh;

        for (int i = 0; i < HBFIRFilterTraits<HBFilterOrder>::hbOrder / 16; i++)
        {
            sh = vld1q_s32(&HBFIRFilterTraits<HBFilterOrder>::hbCoeffs[4*i]);

            if ((m_ptrB % 2) == 0)
            {
                sa = vld1q_s32(&(m_evenA[0][a]));
                sb = vld1q_s32(&(m_evenB[0][b]));
                sumI = vmlaq_s32(sumI, vaddq_s32(sa, sb), sh);

                sa = vld1q_s32(&(m_evenA[1][a]));
                sb = vld1q_s32(&(m_evenB[1][b]));
                sumQ = vmlaq_s32(sumQ, vaddq_s32(sa, sb), sh);
            }
            else
            {
                sa = vld1q_s32(&(m_oddA[0][a]));
                sb = vld1q_s32(&(m_oddB[0][b]));
                sumI = vmlaq_s32(sumI, vaddq_s32(sa, sb), sh);

                sa = vld1q_s32(&(m_oddA[1][a]));
                sb = vld1q_s32(&(m_oddB[1][b]));
                sumQ = vmlaq_s32(sumQ, vaddq_s32(sa, sb), sh);
            }

            a += 4;
            b += 4;
        }

        int32x2_t sumI1 = vpadd_s32(vget_high_s32(sumI), vget_low_s32(sumI));
        int32x2_t sumI2 = vpadd_s32(sumI1, sumI1);
        iAcc = vget_lane_s32(sumI2, 0);

        int32x2_t sumQ1 = vpadd_s32(vget_high_s32(sumQ), vget_low_s32(sumQ));
        int32x2_t sumQ2 = vpadd_s32(sumQ1, sumQ1);
        qAcc = vget_lane_s32(sumQ2, 0);
#else
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
