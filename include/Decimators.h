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
#include "IntHalfbandFilter.h"

class Decimators
{
public:
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
	IntHalfbandFilter m_decimator2;  // 1st stages
	IntHalfbandFilter m_decimator4;  // 2nd stages
	IntHalfbandFilter m_decimator8;  // 3rd stages
	IntHalfbandFilter m_decimator16; // 4th stages
	IntHalfbandFilter m_decimator32; // 5th stages
	IntHalfbandFilter m_decimator64; // 6th stages
};

#endif /* INCLUDE_DECIMATORS_H_ */
