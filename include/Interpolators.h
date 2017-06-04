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

#ifndef INCLUDE_INTERPOLATORS_H_
#define INCLUDE_INTERPOLATORS_H_

#include "SDRDaemon.h"

#if defined(USE_SSE4_1)
#include "IntHalfbandFilterEO1.h"
#else
#include "IntHalfbandFilterDB.h"
#endif

#define INTERPOLATORS_HB_FILTER_ORDER_FIRST  64
#define INTERPOLATORS_HB_FILTER_ORDER_SECOND 32
#define INTERPOLATORS_HB_FILTER_ORDER_NEXT   16

class Interpolators
{
public:
	void interpolate2_cen(const IQSampleVector& in, IQSampleVector& out);
	void interpolate4_cen(const IQSampleVector& in, IQSampleVector& out);
	void interpolate8_cen(const IQSampleVector& in, IQSampleVector& out);
	void interpolate16_cen(const IQSampleVector& in, IQSampleVector& out);
	void interpolate32_cen(const IQSampleVector& in, IQSampleVector& out);
	void interpolate64_cen(const IQSampleVector& in, IQSampleVector& out);

private:
#if defined(USE_SSE4_1)
	IntHalfbandFilterEO1<INTERPOLATORS_HB_FILTER_ORDER_FIRST> m_interpolator2;  // 1st stages
	IntHalfbandFilterEO1<INTERPOLATORS_HB_FILTER_ORDER_SECOND> m_interpolator4;  // 2nd stages
	IntHalfbandFilterEO1<INTERPOLATORS_HB_FILTER_ORDER_NEXT> m_interpolator8;  // 3rd stages
	IntHalfbandFilterEO1<INTERPOLATORS_HB_FILTER_ORDER_NEXT> m_interpolator16; // 4th stages
	IntHalfbandFilterEO1<INTERPOLATORS_HB_FILTER_ORDER_NEXT> m_interpolator32; // 5th stages
	IntHalfbandFilterEO1<INTERPOLATORS_HB_FILTER_ORDER_NEXT> m_interpolator64; // 6th stages
#else
    IntHalfbandFilterDB<INTERPOLATORS_HB_FILTER_ORDER_FIRST> m_interpolator2;  // 1st stages
    IntHalfbandFilterDB<INTERPOLATORS_HB_FILTER_ORDER_SECOND> m_interpolator4;  // 2nd stages
    IntHalfbandFilterDB<INTERPOLATORS_HB_FILTER_ORDER_NEXT> m_interpolator8;  // 3rd stages
    IntHalfbandFilterDB<INTERPOLATORS_HB_FILTER_ORDER_NEXT> m_interpolator16; // 4th stages
    IntHalfbandFilterDB<INTERPOLATORS_HB_FILTER_ORDER_NEXT> m_interpolator32; // 5th stages
    IntHalfbandFilterDB<INTERPOLATORS_HB_FILTER_ORDER_NEXT> m_interpolator64; // 6th stages
#endif
};

#endif /* INCLUDE_INTERPOLATORS_H_ */
