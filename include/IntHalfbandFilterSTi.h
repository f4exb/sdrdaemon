///////////////////////////////////////////////////////////////////////////////////
// Copyright (C) 2016 F4EXB                                                      //
// written by Edouard Griffiths                                                  //
//                                                                               //
// Integer half-band FIR based interpolator and decimator                        //
// This is the even/odd and I/Q stride with double buffering variant             //
// This is the SIMD intrinsics code                                              //
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

#ifndef INCLUDE_INTHALFBANDFILTERSTI_H_
#define INCLUDE_INTHALFBANDFILTERSTI_H_

#include <stdint.h>

#if defined(USE_SSE4_1)
#include <smmintrin.h>
#elif defined(USE_NEON)
#include <arm_neon.h>
#endif

#include "HBFilterTraits.h"

template<uint32_t HBFilterOrder>
class IntHalfbandFilterSTIntrinsics
{
public:
    static void work(
            int32_t samples[HBFilterOrder][2],
            int32_t& iEvenAcc, int32_t& qEvenAcc,
			int32_t& iOddAcc, int32_t& qOddAcc)
    {
#if defined(USE_SSE4_1)
        int a = HBFIRFilterTraits<HBFilterOrder>::hbOrder - 2; // tip
        int b = 0; // tail
        const __m128i* h = (const __m128i*) HBFIRFilterTraits<HBFilterOrder>::hbCoeffs;
        __m128i sum = _mm_setzero_si128();
        __m128i sh, sa, sb;
        int32_t sums[4] __attribute__ ((aligned (16)));

        for (int i = 0; i < HBFIRFilterTraits<HBFilterOrder>::hbOrder / 16; i++)
        {
        	sh = _mm_set_epi32(h[i], h[i], h[i], h[i]);
            sa = _mm_load_si128((__m128i*) &(samples[a][0])); // Ei,Eq,Oi,Oq
            sb = _mm_load_si128((__m128i*) &(samples[b][0]));
            sum = _mm_add_epi32(sum, _mm_mullo_epi32(_mm_add_epi32(sa, sb), sh));
            a -= 2;
            b += 2;
        }

        // Extract values from sum vector
        _mm_store_si128((__m128i*) sums, sum);
        iEvenAcc = sums[0];
        qEvenAcc = sums[1];
        iOddAcc = sums[2];
        qOddAcc = sums[3];
#endif
    }

    // not aligned version
    static void workNA(
            int ptr,
            int32_t samples[HBFilterOrder*2][2],
            int32_t& iEvenAcc, int32_t& qEvenAcc,
            int32_t& iOddAcc, int32_t& qOddAcc)
    {
#if defined(USE_SSE4_1)
        int a = ptr + HBFIRFilterTraits<HBFilterOrder>::hbOrder - 2; // tip
        int b = ptr + 0; // tail
        const __m128i* h = (const __m128i*) HBFIRFilterTraits<HBFilterOrder>::hbCoeffs;
        __m128i sum = _mm_setzero_si128();
        __m128i sh, sa, sb;
        int32_t sums[4] __attribute__ ((aligned (16)));

        for (int i = 0; i < HBFIRFilterTraits<HBFilterOrder>::hbOrder / 4; i++)
        {
        	sh = _mm_set_epi32(h[i], h[i], h[i], h[i]);
            sa = _mm_loadu_si128((__m128i*) &(samples[a][0])); // Ei,Eq,Oi,Oq
            sb = _mm_loadu_si128((__m128i*) &(samples[b][0]));
            sum = _mm_add_epi32(sum, _mm_mullo_epi32(_mm_add_epi32(sa, sb), sh));
            a -= 2;
            b += 2;
        }

        // Extract values from sum vector
        _mm_store_si128((__m128i*) sums, sum);
        iEvenAcc = sums[0];
        qEvenAcc = sums[1];
        iOddAcc = sums[2];
        qOddAcc = sums[3];
#elif defined(USE_NEON)
        int a = ptr + HBFIRFilterTraits<HBFilterOrder>::hbOrder - 2; // tip
        int b = ptr + 0; // tail
        int32x4_t sum = vdupq_n_s32(0);
        int32x4_t sa, sb, sh;
        int32_t sums[4] __attribute__ ((aligned (16)));

        for (int i = 0; i < HBFIRFilterTraits<HBFilterOrder>::hbOrder / 4; i++)
        {
        	sh = vdupq_n_s32(HBFIRFilterTraits<HBFilterOrder>::hbCoeffs[i]);
            sa = vld1q_s32(&(samples[a][0]));
            sb = vld1q_s32(&(samples[b][0]));
            sum = vmlaq_s32(sum, vaddq_s32(sa, sb), sh);
            a -= 2;
            b += 2;
        }

        // Extract values from sum vector
        vst1q_s32(sums, sum);
        iEvenAcc = sums[0];
        qEvenAcc = sums[1];
        iOddAcc = sums[2];
        qOddAcc = sums[3];
#endif
    }
};



#endif /* SDRBASE_DSP_INTHALFBANDFILTERSTI_H_ */
