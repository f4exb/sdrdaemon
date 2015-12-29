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

#include "Decimators.h"

/** double byte samples to double byte samples decimation by 2 low band */
void Decimators::decimate2_inf(unsigned int& sampleSize, const IQSampleVector& in, IQSampleVector& out)
{
	std::size_t len = in.size();
	out.resize(len/2);
	IQSampleVector::iterator it = out.begin();
	unsigned int decimation_shift = (sampleSize < 15 ? 0 : sampleSize - 15);

	std::int32_t xreal, yimag;

	for (unsigned int pos = 0; pos < len - 3; pos += 4)
	{
		xreal = in[pos+0].real() - in[pos+1].imag();
		yimag = in[pos+0].imag() + in[pos+1].real();
        it->setReal(xreal >> decimation_shift);
        it->setImag(yimag >> decimation_shift);
        ++it;
        xreal =  in[pos+3].imag() - in[pos+2].real();
        yimag = -in[pos+2].imag() - in[pos+3].real();
        it->setReal(xreal >> decimation_shift);
        it->setImag(yimag >> decimation_shift);
        ++it;
	}

	sampleSize += (1 - decimation_shift);
}

/** double byte samples to double byte samples decimation by 2 high band */
void Decimators::decimate2_sup(unsigned int& sampleSize, const IQSampleVector& in, IQSampleVector& out)
{
	std::size_t len = in.size();
	out.resize(len/2);
	IQSampleVector::iterator it = out.begin();
	unsigned int decimation_shift = (sampleSize < 15 ? 0 : sampleSize - 15);

	std::int32_t xreal, yimag;

	for (unsigned int pos = 0; pos < len - 3; pos += 4)
	{
		xreal =  in[pos+0].imag() - in[pos+1].real();
		yimag = -in[pos+0].real() - in[pos+1].imag();
        it->setReal(xreal >> decimation_shift);
        it->setImag(yimag >> decimation_shift);
        ++it;
        xreal = in[pos+3].real() - in[pos+2].imag();
        yimag = in[pos+2].real() + in[pos+3].imag();
        it->setReal(xreal >> decimation_shift);
        it->setImag(yimag >> decimation_shift);
        ++it;
	}

	sampleSize += (1 - decimation_shift);
}

/** double byte samples to double byte samples decimation by 2 centered */
void Decimators::decimate2_cen(unsigned int& sampleSize, const IQSampleVector& in, IQSampleVector& out)
{
	std::size_t len = in.size();
	out.resize(len/2);
	IQSampleVector::iterator it = out.begin();
	unsigned int decimation_shift = (sampleSize < 15 ? 0 : sampleSize - 15);
	int32_t intbuf[2];

	for (unsigned int pos = 0; pos < len - 1; pos += 2)
	{
		intbuf[0]  = in[pos+1].real();
		intbuf[1]  = in[pos+1].imag();

		m_decimator2.myDecimate(
				in[pos+0].real(),
				in[pos+0].imag(),
				&intbuf[0],
				&intbuf[1]);

		it->setReal(intbuf[0] >> decimation_shift);
		it->setImag(intbuf[1] >> decimation_shift);
		++it;
	}

	sampleSize += (1 - decimation_shift);
}

/** double byte samples to double byte samples decimation by 4 low band
 * Inf (LSB):
 *            x  y   x  y   x   y  x   y  / x -> 0,-3,-4,7 / y -> 1,2,-5,-6
 * [ rotate:  0, 1, -3, 2, -4, -5, 7, -6]
 */
void Decimators::decimate4_inf(unsigned int& sampleSize, const IQSampleVector& in, IQSampleVector& out)
{
	std::size_t len = in.size();
	out.resize(len/4);
	IQSampleVector::iterator it = out.begin();
	unsigned int decimation_shift = (sampleSize < 14 ? 0 : sampleSize - 14);

	std::int32_t xreal, yimag;

	for (unsigned int pos = 0; pos < len - 3; pos += 4)
	{
		xreal = in[pos+0].real() - in[pos+1].imag() + in[pos+3].imag() - in[pos+2].real();
		yimag = in[pos+0].imag() - in[pos+2].imag() + in[pos+1].real() - in[pos+3].real();
        it->setReal(xreal >> decimation_shift);
        it->setImag(yimag >> decimation_shift);
        ++it;
	}

	sampleSize += (2 - decimation_shift);
}

/** double byte samples to double byte samples decimation by 4 high band */
void Decimators::decimate4_sup(unsigned int& sampleSize, const IQSampleVector& in, IQSampleVector& out)
{
	std::size_t len = in.size();
	out.resize(len/4);
	IQSampleVector::iterator it = out.begin();
	unsigned int decimation_shift = (sampleSize < 14 ? 0 : sampleSize - 14);

	std::int32_t xreal, yimag;

	for (unsigned int pos = 0; pos < len - 3; pos += 4)
	{
		xreal =  in[pos+0].imag() - in[pos+1].real() - in[pos+2].imag() + in[pos+3].real();
		yimag = -in[pos+0].real() - in[pos+1].imag() + in[pos+2].real() + in[pos+3].imag();
        it->setReal(xreal >> decimation_shift);
        it->setImag(yimag >> decimation_shift);
        ++it;
	}

	sampleSize += (2 - decimation_shift);
}

/** double byte samples to double byte samples decimation by 4 centered */
void Decimators::decimate4_cen(unsigned int& sampleSize, const IQSampleVector& in, IQSampleVector& out)
{
	std::size_t len = in.size();
	out.resize(len/4);
	IQSampleVector::iterator it = out.begin();
	unsigned int decimation_shift = (sampleSize < 14 ? 0 : sampleSize - 14);
	int32_t intbuf[4];

	for (unsigned int pos = 0; pos < len - 3; pos += 4)
	{
		intbuf[0]  = in[pos+1].real();
		intbuf[1]  = in[pos+1].imag();
		intbuf[2]  = in[pos+3].real();
		intbuf[3]  = in[pos+3].imag();

		m_decimator2.myDecimate(
				in[pos+0].real(),
				in[pos+0].imag(),
				&intbuf[0],
				&intbuf[1]);

		m_decimator2.myDecimate(
				in[pos+2].real(),
				in[pos+2].imag(),
				&intbuf[2],
				&intbuf[3]);

		m_decimator4.myDecimate(
				intbuf[0],
				intbuf[1],
				&intbuf[2],
				&intbuf[3]);

		it->setReal(intbuf[2] >> decimation_shift);
		it->setImag(intbuf[3] >> decimation_shift);
		++it;
	}

	sampleSize += (2 - decimation_shift);
}

/** double byte samples to double byte samples decimation by 8 low band */
void Decimators::decimate8_inf(unsigned int& sampleSize, const IQSampleVector& in, IQSampleVector& out)
{
	std::size_t len = in.size();
	out.resize(len/8);
	IQSampleVector::iterator it = out.begin();
	unsigned int decimation_shift = (sampleSize < 13 ? 0 : sampleSize - 13);

	std::int32_t xreal[2], yimag[2];

	for (unsigned int pos = 0; pos < len - 7; pos += 4)
	{
		xreal[0] = in[pos+0].real() - in[pos+1].imag() + in[pos+3].imag() - in[pos+2].real();
		yimag[0] = in[pos+0].imag() - in[pos+2].imag() + in[pos+1].real() - in[pos+3].real();
		pos += 4;
		xreal[1] = in[pos+0].real() - in[pos+1].imag() + in[pos+3].imag() - in[pos+2].real();
		yimag[1] = in[pos+0].imag() - in[pos+2].imag() + in[pos+1].real() - in[pos+3].real();
		m_decimator2.myDecimate(xreal[0], yimag[0], &xreal[1], &yimag[1]);
        it->setReal(xreal[1] >> decimation_shift);
        it->setImag(yimag[1] >> decimation_shift);
        ++it;
	}

	sampleSize += (3 - decimation_shift);
}

/** double byte samples to double byte samples decimation by 8 high band */
void Decimators::decimate8_sup(unsigned int& sampleSize, const IQSampleVector& in, IQSampleVector& out)
{
	std::size_t len = in.size();
	out.resize(len/8);
	IQSampleVector::iterator it = out.begin();
	unsigned int decimation_shift = (sampleSize < 13 ? 0 : sampleSize - 13);

	std::int32_t xreal[2], yimag[2];

	for (unsigned int pos = 0; pos < len - 7; pos += 4)
	{
		xreal[0] =  in[pos+0].imag() - in[pos+1].real() - in[pos+2].imag() + in[pos+3].real();
		yimag[0] = -in[pos+0].real() - in[pos+1].imag() + in[pos+2].real() + in[pos+3].imag();
		pos += 4;
		xreal[1] =  in[pos+0].imag() - in[pos+1].real() - in[pos+2].imag() + in[pos+3].real();
		yimag[1] = -in[pos+0].real() - in[pos+1].imag() + in[pos+2].real() + in[pos+3].imag();
		m_decimator2.myDecimate(xreal[0], yimag[0], &xreal[1], &yimag[1]);
        it->setReal(xreal[1] >> decimation_shift);
        it->setImag(yimag[1] >> decimation_shift);
        ++it;
	}

	sampleSize += (3 - decimation_shift);
}

/** double byte samples to double byte samples decimation by 8 centered */
void Decimators::decimate8_cen(unsigned int& sampleSize, const IQSampleVector& in, IQSampleVector& out)
{
	std::size_t len = in.size();
	out.resize(len/8);
	IQSampleVector::iterator it = out.begin();
	unsigned int decimation_shift = (sampleSize < 13 ? 0 : sampleSize - 13);
	int32_t intbuf[8];

	for (unsigned int pos = 0; pos < len - 7; pos += 8)
	{
		intbuf[0]  = in[pos+1].real();
		intbuf[1]  = in[pos+1].imag();
		intbuf[2]  = in[pos+3].real();
		intbuf[3]  = in[pos+3].imag();
		intbuf[4]  = in[pos+5].real();
		intbuf[5]  = in[pos+5].imag();
		intbuf[6]  = in[pos+7].real();
		intbuf[7]  = in[pos+7].imag();

		m_decimator2.myDecimate(
				in[pos+0].real(),
				in[pos+0].imag(),
				&intbuf[0],
				&intbuf[1]);
		m_decimator2.myDecimate(
				in[pos+2].real(),
				in[pos+2].imag(),
				&intbuf[2],
				&intbuf[3]);
		m_decimator2.myDecimate(
				in[pos+4].real(),
				in[pos+4].imag(),
				&intbuf[4],
				&intbuf[5]);
		m_decimator2.myDecimate(
				in[pos+6].real(),
				in[pos+6].imag(),
				&intbuf[6],
				&intbuf[7]);

		m_decimator4.myDecimate(
				intbuf[0],
				intbuf[1],
				&intbuf[2],
				&intbuf[3]);
		m_decimator4.myDecimate(
				intbuf[4],
				intbuf[5],
				&intbuf[6],
				&intbuf[7]);

		m_decimator8.myDecimate(
				intbuf[2],
				intbuf[3],
				&intbuf[6],
				&intbuf[7]);

		it->setReal(intbuf[6] >> decimation_shift);
		it->setImag(intbuf[7] >> decimation_shift);
		++it;
	}

	sampleSize += (3 - decimation_shift);
}
