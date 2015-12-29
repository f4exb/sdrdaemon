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

#ifndef INCLUDE_INTHALFBANDFILTER_H_
#define INCLUDE_INTHALFBANDFILTER_H_

#include <cstdint>

/*
 * supported filter orders: 64, 48, 32
 */
#define HB_FILTERORDER 32
#define HB_SHIFT 14

/** Slimmed out class from SDRangel's IntHalfbandFilter */
class IntHalfbandFilter
{
public:
	IntHalfbandFilter();

	void myDecimate(int32_t x1, int32_t y1, int32_t *x2, int32_t *y2)
	{
		m_samples[m_ptr][0] = x1;
		m_samples[m_ptr][1] = y1;
		m_ptr = mod33[m_ptr + 2 - 1];

		m_samples[m_ptr][0] = *x2;
		m_samples[m_ptr][1] = *y2;

		doFIR(x2, y2);

		m_ptr = mod33[m_ptr + 2 - 1];
	}

protected:
	int32_t m_samples[HB_FILTERORDER + 1][2];
	int16_t m_ptr;
	int m_state;

	// coefficents
#if HB_FILTERORDER == 64
	static const int32_t COEFF[16];
#elif HB_FILTERORDER == 48
	static const int32_t COEFF[12];
#elif HB_FILTERORDER == 32
	static const int32_t mod33[38];
	static const int32_t COEFF[8];
#else
#error unsupported filter order
#endif

	void doFIR(int32_t *x, int32_t *y)
	{
		// init read-pointer
		int a = mod33[m_ptr + 2 + 1]; // 0 + 1
		int b = mod33[m_ptr + 2 - 2]; //-1 - 1

		// go through samples in buffer
		int32_t iAcc = 0;
		int32_t qAcc = 0;

		for (int i = 0; i < HB_FILTERORDER / 4; i++)
		{
			// do multiply-accumulate
			//qint32 iTmp = m_samples[a][0] + m_samples[b][0]; // Valgrind optim
			//qint32 qTmp = m_samples[a][1] + m_samples[b][1]; // Valgrind optim
			iAcc += (m_samples[a][0] + m_samples[b][0]) * COEFF[i];
			qAcc += (m_samples[a][1] + m_samples[b][1]) * COEFF[i];

			// update read-pointer
			a = mod33[a + 2 + 2];
			b = mod33[b + 2 - 2];
		}

		a = mod33[a + 2 - 1];

		iAcc += ((int32_t)m_samples[a][0] + 1) << (HB_SHIFT - 1);
		qAcc += ((int32_t)m_samples[a][1] + 1) << (HB_SHIFT - 1);

		*x = iAcc >> (HB_SHIFT -1); // HB_SHIFT incorrect do not loose the gained bit
		*y = qAcc >> (HB_SHIFT -1);
	}
};



#endif /* INCLUDE_INTHALFBANDFILTER_H_ */
