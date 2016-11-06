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

#ifndef INCLUDE_INTHALFBANDFILTEREO2I_H_
#define INCLUDE_INTHALFBANDFILTEREO2I_H_

#include <stdint.h>

#ifdef USE_SSE4_1
#include <smmintrin.h>
#endif

#ifdef USE_NEON
#include <arm_neon.h>
#endif

#include "HBFilterTraits.h"

template<uint32_t HBFilterOrder>
class IntHalfbandFilterEO2Intrisics
{
public:
    static void work(
            int ptrA,
            int ptrB,
            int32_t evenA[2][HBFilterOrder],
            int32_t evenB[2][HBFilterOrder],
            int32_t oddA[2][HBFilterOrder],
            int32_t oddB[2][HBFilterOrder],
            int32_t& iAcc, int32_t& qAcc)
    {
#if defined(USE_SSE4_1)
        int a = ptrA/2; // tip pointer
        int b = ptrB/2 + 1; // tail pointer
        const __m128i* h = (const __m128i*) HBFIRFilterTraits<HBFilterOrder>::hbCoeffs;
        __m128i sumI = _mm_setzero_si128();
        __m128i sumQ = _mm_setzero_si128();
        __m128i sa, sb;

        for (int i = 0; i < HBFIRFilterTraits<HBFilterOrder>::hbOrder / 16; i++)
        {
            if ((ptrB % 2) == 0)
            {
                sa = _mm_loadu_si128((__m128i*) &(evenA[0][a]));
                sb = _mm_loadu_si128((__m128i*) &(evenB[0][b]));
                sumI = _mm_add_epi32(sumI, _mm_mullo_epi32(_mm_add_epi32(sa, sb), *h));

                sa = _mm_loadu_si128((__m128i*) &(evenA[1][a]));
                sb = _mm_loadu_si128((__m128i*) &(evenB[1][b]));
                sumQ = _mm_add_epi32(sumQ, _mm_mullo_epi32(_mm_add_epi32(sa, sb), *h));
            }
            else
            {
                sa = _mm_loadu_si128((__m128i*) &(oddA[0][a]));
                sb = _mm_loadu_si128((__m128i*) &(oddB[0][b]));
                sumI = _mm_add_epi32(sumI, _mm_mullo_epi32(_mm_add_epi32(sa, sb), *h));

                sa = _mm_loadu_si128((__m128i*) &(oddA[1][a]));
                sb = _mm_loadu_si128((__m128i*) &(oddB[1][b]));
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
        int a = ptrA/2; // tip pointer
        int b = ptrB/2 + 1; // tail pointer
        int32x4_t sumI = vdupq_n_s32(0);
        int32x4_t sumQ = vdupq_n_s32(0);
        int32x4_t sa, sb, sh;

        for (int i = 0; i < HBFIRFilterTraits<HBFilterOrder>::hbOrder / 16; i++)
        {
            sh = vld1q_s32(&HBFIRFilterTraits<HBFilterOrder>::hbCoeffs[4*i]);

            if ((ptrB % 2) == 0)
            {
                sa = vld1q_s32(&(evenA[0][a]));
                sb = vld1q_s32(&(evenB[0][b]));
                sumI = vmlaq_s32(sumI, vaddq_s32(sa, sb), sh);

                sa = vld1q_s32(&(evenA[1][a]));
                sb = vld1q_s32(&(evenB[1][b]));
                sumQ = vmlaq_s32(sumQ, vaddq_s32(sa, sb), sh);
            }
            else
            {
                sa = vld1q_s32(&(oddA[0][a]));
                sb = vld1q_s32(&(oddB[0][b]));
                sumI = vmlaq_s32(sumI, vaddq_s32(sa, sb), sh);

                sa = vld1q_s32(&(oddA[1][a]));
                sb = vld1q_s32(&(oddB[1][b]));
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
#endif
    }
};

template<>
class IntHalfbandFilterEO2Intrisics<48>
{
public:
    static void work(
            int ptrA,
            int ptrB,
            int32_t evenA[2][48],
            int32_t evenB[2][48],
            int32_t oddA[2][48],
            int32_t oddB[2][48],
            int32_t& iAcc, int32_t& qAcc)
    {
#if defined(USE_SSE4_1)
        int a = ptrA/2; // tip pointer
        int b = ptrB/2 + 1; // tail pointer
        const __m128i* h = (const __m128i*) HBFIRFilterTraits<48>::hbCoeffs;
        __m128i sumI = _mm_setzero_si128();
        __m128i sumQ = _mm_setzero_si128();
        __m128i sa, sb;

        if ((ptrB % 2) == 0)
        {
            // 0
            sa = _mm_loadu_si128((__m128i*) &(evenA[0][a]));
            sb = _mm_loadu_si128((__m128i*) &(evenB[0][b]));
            sumI = _mm_add_epi32(sumI, _mm_mullo_epi32(_mm_add_epi32(sa, sb), *h));
            sa = _mm_loadu_si128((__m128i*) &(evenA[1][a]));
            sb = _mm_loadu_si128((__m128i*) &(evenB[1][b]));
            sumQ = _mm_add_epi32(sumQ, _mm_mullo_epi32(_mm_add_epi32(sa, sb), *h));
            h++;
            // 1
            sa = _mm_loadu_si128((__m128i*) &(evenA[0][a+4]));
            sb = _mm_loadu_si128((__m128i*) &(evenB[0][b+4]));
            sumI = _mm_add_epi32(sumI, _mm_mullo_epi32(_mm_add_epi32(sa, sb), *h));
            sa = _mm_loadu_si128((__m128i*) &(evenA[1][a+4]));
            sb = _mm_loadu_si128((__m128i*) &(evenB[1][b+4]));
            sumQ = _mm_add_epi32(sumQ, _mm_mullo_epi32(_mm_add_epi32(sa, sb), *h));
            h++;
            // 2
            sa = _mm_loadu_si128((__m128i*) &(evenA[0][a+8]));
            sb = _mm_loadu_si128((__m128i*) &(evenB[0][b+8]));
            sumI = _mm_add_epi32(sumI, _mm_mullo_epi32(_mm_add_epi32(sa, sb), *h));
            sa = _mm_loadu_si128((__m128i*) &(evenA[1][a+8]));
            sb = _mm_loadu_si128((__m128i*) &(evenB[1][b+8]));
            sumQ = _mm_add_epi32(sumQ, _mm_mullo_epi32(_mm_add_epi32(sa, sb), *h));
        }
        else
        {
            // 0
            sa = _mm_loadu_si128((__m128i*) &(oddA[0][a]));
            sb = _mm_loadu_si128((__m128i*) &(oddB[0][b]));
            sumI = _mm_add_epi32(sumI, _mm_mullo_epi32(_mm_add_epi32(sa, sb), *h));
            sa = _mm_loadu_si128((__m128i*) &(oddA[1][a]));
            sb = _mm_loadu_si128((__m128i*) &(oddB[1][b]));
            sumQ = _mm_add_epi32(sumQ, _mm_mullo_epi32(_mm_add_epi32(sa, sb), *h));
            h++;
            // 1
            sa = _mm_loadu_si128((__m128i*) &(oddA[0][a+4]));
            sb = _mm_loadu_si128((__m128i*) &(oddB[0][b+4]));
            sumI = _mm_add_epi32(sumI, _mm_mullo_epi32(_mm_add_epi32(sa, sb), *h));
            sa = _mm_loadu_si128((__m128i*) &(oddA[1][a+4]));
            sb = _mm_loadu_si128((__m128i*) &(oddB[1][b+4]));
            sumQ = _mm_add_epi32(sumQ, _mm_mullo_epi32(_mm_add_epi32(sa, sb), *h));
            h++;
            // 2
            sa = _mm_loadu_si128((__m128i*) &(oddA[0][a+8]));
            sb = _mm_loadu_si128((__m128i*) &(oddB[0][b+8]));
            sumI = _mm_add_epi32(sumI, _mm_mullo_epi32(_mm_add_epi32(sa, sb), *h));
            sa = _mm_loadu_si128((__m128i*) &(oddA[1][a+8]));
            sb = _mm_loadu_si128((__m128i*) &(oddB[1][b+8]));
            sumQ = _mm_add_epi32(sumQ, _mm_mullo_epi32(_mm_add_epi32(sa, sb), *h));
        }

#elif defined(USE_NEON)
        int a = ptrA/2; // tip pointer
        int b = ptrB/2 + 1; // tail pointer
        int32x4_t sumI = vdupq_n_s32(0);
        int32x4_t sumQ = vdupq_n_s32(0);
        int32x4x3_t sh = vld3q_s32((int32_t const *) &HBFIRFilterTraits<48>::hbCoeffs[0]);
        int32x4x3_t sa, sb;

        if ((ptrB % 2) == 0)
        {
            sa = vld3q_s32((int32_t const *) &(evenA[0][a]));
            sb = vld3q_s32((int32_t const *) &(evenB[0][b]));
            sumI = vmlaq_s32(sumI, vaddq_s32(sa.val[0], sb.val[0]), sh.val[0]);
            sumI = vmlaq_s32(sumI, vaddq_s32(sa.val[1], sb.val[1]), sh.val[1]);
            sumI = vmlaq_s32(sumI, vaddq_s32(sa.val[2], sb.val[2]), sh.val[2]);
            sa = vld3q_s32((int32_t const *) &(evenA[1][a]));
            sb = vld3q_s32((int32_t const *) &(evenB[1][b]));
            sumQ = vmlaq_s32(sumQ, vaddq_s32(sa.val[0], sb.val[0]), sh.val[0]);
            sumQ = vmlaq_s32(sumQ, vaddq_s32(sa.val[1], sb.val[1]), sh.val[1]);
            sumQ = vmlaq_s32(sumQ, vaddq_s32(sa.val[2], sb.val[2]), sh.val[2]);
        }
        else
        {
            sa = vld3q_s32((int32_t const *) &(oddA[0][a]));
            sb = vld3q_s32((int32_t const *) &(oddB[0][b]));
            sumI = vmlaq_s32(sumI, vaddq_s32(sa.val[0], sb.val[0]), sh.val[0]);
            sumI = vmlaq_s32(sumI, vaddq_s32(sa.val[1], sb.val[1]), sh.val[1]);
            sumI = vmlaq_s32(sumI, vaddq_s32(sa.val[2], sb.val[2]), sh.val[2]);
            sa = vld3q_s32((int32_t const *) &(oddA[1][a]));
            sb = vld3q_s32((int32_t const *) &(oddB[1][b]));
            sumQ = vmlaq_s32(sumQ, vaddq_s32(sa.val[0], sb.val[0]), sh.val[0]);
            sumQ = vmlaq_s32(sumQ, vaddq_s32(sa.val[1], sb.val[1]), sh.val[1]);
            sumQ = vmlaq_s32(sumQ, vaddq_s32(sa.val[2], sb.val[2]), sh.val[2]);
        }

        int32x2_t sumI1 = vpadd_s32(vget_high_s32(sumI), vget_low_s32(sumI));
        int32x2_t sumI2 = vpadd_s32(sumI1, sumI1);
        iAcc = vget_lane_s32(sumI2, 0);

        int32x2_t sumQ1 = vpadd_s32(vget_high_s32(sumQ), vget_low_s32(sumQ));
        int32x2_t sumQ2 = vpadd_s32(sumQ1, sumQ1);
        qAcc = vget_lane_s32(sumQ2, 0);
#endif
    }
};

#endif /* INCLUDE_INTHALFBANDFILTEREO2I_H_ */
