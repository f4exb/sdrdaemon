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

#ifndef INCLUDE_DECIMATORS_H_
#define INCLUDE_DECIMATORS_H_

#include "SDRDaemon.h"

#if defined(USE_SSE4_1)
#include "IntHalfbandFilterEO1.h"
#else
#include "IntHalfbandFilterDB.h"
#endif

#define DECIMATORS_HB_FILTER_ORDER 64

class Decimators
{
public:
	static void decimate1(unsigned int& sampleSize, IQSampleVector& inout);
	static void decimate2_inf(unsigned int& sampleSize, const IQSampleVector& in, IQSampleVector& out);
	static void decimate2_sup(unsigned int& sampleSize, const IQSampleVector& in, IQSampleVector& out);
	void decimate2_cen(unsigned int& sampleSize, const IQSampleVector& in, IQSampleVector& out);
	static void decimate4_inf(unsigned int& sampleSize, const IQSampleVector& in, IQSampleVector& out);
	static void decimate4_sup(unsigned int& sampleSize, const IQSampleVector& in, IQSampleVector& out);
	void decimate4_cen(unsigned int& sampleSize, const IQSampleVector& in, IQSampleVector& out);
	void decimate8_inf(unsigned int& sampleSize, const IQSampleVector& in, IQSampleVector& out);
	void decimate8_sup(unsigned int& sampleSize, const IQSampleVector& in, IQSampleVector& out);
	void decimate8_cen(unsigned int& sampleSize, const IQSampleVector& in, IQSampleVector& out);
	void decimate16_inf(unsigned int& sampleSize, const IQSampleVector& in, IQSampleVector& out);
	void decimate16_sup(unsigned int& sampleSize, const IQSampleVector& in, IQSampleVector& out);
	void decimate16_cen(unsigned int& sampleSize, const IQSampleVector& in, IQSampleVector& out);
	void decimate32_inf(unsigned int& sampleSize, const IQSampleVector& in, IQSampleVector& out);
	void decimate32_sup(unsigned int& sampleSize, const IQSampleVector& in, IQSampleVector& out);
	void decimate32_cen(unsigned int& sampleSize, const IQSampleVector& in, IQSampleVector& out);
	void decimate64_inf(unsigned int& sampleSize, const IQSampleVector& in, IQSampleVector& out);
	void decimate64_sup(unsigned int& sampleSize, const IQSampleVector& in, IQSampleVector& out);
	void decimate64_cen(unsigned int& sampleSize, const IQSampleVector& in, IQSampleVector& out);

private:
#if defined(USE_SSE4_1)
	IntHalfbandFilterEO1<DECIMATORS_HB_FILTER_ORDER> m_decimator2;  // 1st stages
	IntHalfbandFilterEO1<DECIMATORS_HB_FILTER_ORDER> m_decimator4;  // 2nd stages
	IntHalfbandFilterEO1<DECIMATORS_HB_FILTER_ORDER> m_decimator8;  // 3rd stages
	IntHalfbandFilterEO1<DECIMATORS_HB_FILTER_ORDER> m_decimator16; // 4th stages
	IntHalfbandFilterEO1<DECIMATORS_HB_FILTER_ORDER> m_decimator32; // 5th stages
	IntHalfbandFilterEO1<DECIMATORS_HB_FILTER_ORDER> m_decimator64; // 6th stages
#else
    IntHalfbandFilterDB<DECIMATORS_HB_FILTER_ORDER> m_decimator2;  // 1st stages
    IntHalfbandFilterDB<DECIMATORS_HB_FILTER_ORDER> m_decimator4;  // 2nd stages
    IntHalfbandFilterDB<DECIMATORS_HB_FILTER_ORDER> m_decimator8;  // 3rd stages
    IntHalfbandFilterDB<DECIMATORS_HB_FILTER_ORDER> m_decimator16; // 4th stages
    IntHalfbandFilterDB<DECIMATORS_HB_FILTER_ORDER> m_decimator32; // 5th stages
    IntHalfbandFilterDB<DECIMATORS_HB_FILTER_ORDER> m_decimator64; // 6th stages
#endif
};

#endif /* INCLUDE_DECIMATORS_H_ */
