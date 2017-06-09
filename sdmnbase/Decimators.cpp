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

/** Just do a rescaling to 16 bits */
void Decimators::decimate1(unsigned int& sampleSize, IQSampleVector& inout)
{
	if (sampleSize < 16) // else do nothing
	{
		std::size_t len = inout.size();
		unsigned int norm_shift = 16 - sampleSize; // shift to normalize to 16 bits (shift left)

		for (unsigned int pos = 0; pos < len; pos += 1)
		{
			inout[pos].setReal(inout[pos].real()<<norm_shift);
			inout[pos].setImag(inout[pos].imag()<<norm_shift);
		}
	}
}

/** double byte samples to double byte samples decimation by 2 low band */
void Decimators::decimate2_inf(unsigned int& sampleSize, const IQSampleVector& in, IQSampleVector& out)
{
	std::size_t len = in.size();
	out.resize(len/2);
	IQSampleVector::iterator it = out.begin();
	unsigned int trunk_shift = (sampleSize < 15 ? 0 : sampleSize - 15); // trunk to keep 16 bits (shift right)
	unsigned int norm_shift  = (sampleSize < 15 ? 15 - sampleSize : 0); // shift to normalize to 16 bits (shift left)

	std::int32_t xreal, yimag;

	for (unsigned int pos = 0; pos < len - 3; pos += 4)
	{
		xreal = in[pos+0].real() - in[pos+1].imag();
		yimag = in[pos+0].imag() + in[pos+1].real();
        it->setReal(xreal << norm_shift >> trunk_shift);
        it->setImag(yimag << norm_shift >> trunk_shift);
        ++it;
        xreal =  in[pos+3].imag() - in[pos+2].real();
        yimag = -in[pos+2].imag() - in[pos+3].real();
        it->setReal(xreal << norm_shift >> trunk_shift);
        it->setImag(yimag << norm_shift >> trunk_shift);
        ++it;
	}

	sampleSize += (1 - trunk_shift);
}

/** double byte samples to double byte samples decimation by 2 high band */
void Decimators::decimate2_sup(unsigned int& sampleSize, const IQSampleVector& in, IQSampleVector& out)
{
	std::size_t len = in.size();
	out.resize(len/2);
	IQSampleVector::iterator it = out.begin();
	unsigned int trunk_shift = (sampleSize < 15 ? 0 : sampleSize - 15); // trunk to keep 16 bits (shift right)
	unsigned int norm_shift  = (sampleSize < 15 ? 15 - sampleSize : 0); // shift to normalize to 16 bits (shift left)

	std::int32_t xreal, yimag;

	for (unsigned int pos = 0; pos < len - 3; pos += 4)
	{
		xreal =  in[pos+0].imag() - in[pos+1].real();
		yimag = -in[pos+0].real() - in[pos+1].imag();
        it->setReal(xreal << norm_shift >> trunk_shift);
        it->setImag(yimag << norm_shift >> trunk_shift);
        ++it;
        xreal = in[pos+3].real() - in[pos+2].imag();
        yimag = in[pos+2].real() + in[pos+3].imag();
        it->setReal(xreal << norm_shift >> trunk_shift);
        it->setImag(yimag << norm_shift >> trunk_shift);
        ++it;
	}

	sampleSize += (1 - trunk_shift);
}

/** double byte samples to double byte samples decimation by 2 centered */
void Decimators::decimate2_cen(unsigned int& sampleSize, const IQSampleVector& in, IQSampleVector& out)
{
	std::size_t len = in.size();
	out.resize(len/2);
	IQSampleVector::iterator it = out.begin();
	unsigned int trunk_shift = (sampleSize < 15 ? 0 : sampleSize - 15); // trunk to keep 16 bits (shift right)
	unsigned int norm_shift  = (sampleSize < 15 ? 15 - sampleSize : 0); // shift to normalize to 16 bits (shift left)
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

		it->setReal(intbuf[0] << norm_shift >> trunk_shift);
		it->setImag(intbuf[1] << norm_shift >> trunk_shift);
		++it;
	}

	sampleSize += (1 - trunk_shift);
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
	unsigned int trunk_shift = (sampleSize < 14 ? 0 : sampleSize - 14); // trunk to keep 16 bits (shift right)
	unsigned int norm_shift  = (sampleSize < 14 ? 14 - sampleSize : 0); // shift to normalize to 16 bits (shift left)

	std::int32_t xreal, yimag;

	for (unsigned int pos = 0; pos < len - 3; pos += 4)
	{
		xreal = in[pos+0].real() - in[pos+1].imag() + in[pos+3].imag() - in[pos+2].real();
		yimag = in[pos+0].imag() - in[pos+2].imag() + in[pos+1].real() - in[pos+3].real();
        it->setReal(xreal << norm_shift >> trunk_shift);
        it->setImag(yimag << norm_shift >> trunk_shift);
        ++it;
	}

	sampleSize += (2 - trunk_shift);
}

/** double byte samples to double byte samples decimation by 4 high band */
void Decimators::decimate4_sup(unsigned int& sampleSize, const IQSampleVector& in, IQSampleVector& out)
{
	std::size_t len = in.size();
	out.resize(len/4);
	IQSampleVector::iterator it = out.begin();
	unsigned int trunk_shift = (sampleSize < 14 ? 0 : sampleSize - 14); // trunk to keep 16 bits (shift right)
	unsigned int norm_shift  = (sampleSize < 14 ? 14 - sampleSize : 0); // shift to normalize to 16 bits (shift left)

	std::int32_t xreal, yimag;

	for (unsigned int pos = 0; pos < len - 3; pos += 4)
	{
		xreal =  in[pos+0].imag() - in[pos+1].real() - in[pos+2].imag() + in[pos+3].real();
		yimag = -in[pos+0].real() - in[pos+1].imag() + in[pos+2].real() + in[pos+3].imag();
        it->setReal(xreal << norm_shift >> trunk_shift);
        it->setImag(yimag << norm_shift >> trunk_shift);
        ++it;
	}

	sampleSize += (2 - trunk_shift);
}

/** double byte samples to double byte samples decimation by 4 centered */
void Decimators::decimate4_cen(unsigned int& sampleSize, const IQSampleVector& in, IQSampleVector& out)
{
	std::size_t len = in.size();
	out.resize(len/4);
	IQSampleVector::iterator it = out.begin();
	unsigned int trunk_shift = (sampleSize < 14 ? 0 : sampleSize - 14); // trunk to keep 16 bits (shift right)
	unsigned int norm_shift  = (sampleSize < 14 ? 14 - sampleSize : 0); // shift to normalize to 16 bits (shift left)
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

		it->setReal(intbuf[2] << norm_shift >> trunk_shift);
		it->setImag(intbuf[3] << norm_shift >> trunk_shift);
		++it;
	}

	sampleSize += (2 - trunk_shift);
}

/** double byte samples to double byte samples decimation by 8 low band */
void Decimators::decimate8_inf(unsigned int& sampleSize, const IQSampleVector& in, IQSampleVector& out)
{
	std::size_t len = in.size();
	out.resize(len/8);
	IQSampleVector::iterator it = out.begin();
	unsigned int trunk_shift = (sampleSize < 13 ? 0 : sampleSize - 13); // trunk to keep 16 bits (shift right)
	unsigned int norm_shift  = (sampleSize < 13 ? 13 - sampleSize : 0); // shift to normalize to 16 bits (shift left)

	std::int32_t xreal[2], yimag[2];

	for (unsigned int pos = 0; pos < len - 7; pos += 4)
	{
		xreal[0] = in[pos+0].real() - in[pos+1].imag() + in[pos+3].imag() - in[pos+2].real();
		yimag[0] = in[pos+0].imag() - in[pos+2].imag() + in[pos+1].real() - in[pos+3].real();
		pos += 4;
		xreal[1] = in[pos+0].real() - in[pos+1].imag() + in[pos+3].imag() - in[pos+2].real();
		yimag[1] = in[pos+0].imag() - in[pos+2].imag() + in[pos+1].real() - in[pos+3].real();
		m_decimator2.myDecimate(xreal[0], yimag[0], &xreal[1], &yimag[1]);
        it->setReal(xreal[1] << norm_shift >> trunk_shift);
        it->setImag(yimag[1] << norm_shift >> trunk_shift);
        ++it;
	}

	sampleSize += (3 - trunk_shift);
}

/** double byte samples to double byte samples decimation by 8 high band */
void Decimators::decimate8_sup(unsigned int& sampleSize, const IQSampleVector& in, IQSampleVector& out)
{
	std::size_t len = in.size();
	out.resize(len/8);
	IQSampleVector::iterator it = out.begin();
	unsigned int trunk_shift = (sampleSize < 13 ? 0 : sampleSize - 13); // trunk to keep 16 bits (shift right)
	unsigned int norm_shift  = (sampleSize < 13 ? 13 - sampleSize : 0); // shift to normalize to 16 bits (shift left)

	std::int32_t xreal[2], yimag[2];

	for (unsigned int pos = 0; pos < len - 7; pos += 4)
	{
		xreal[0] =  in[pos+0].imag() - in[pos+1].real() - in[pos+2].imag() + in[pos+3].real();
		yimag[0] = -in[pos+0].real() - in[pos+1].imag() + in[pos+2].real() + in[pos+3].imag();
		pos += 4;
		xreal[1] =  in[pos+0].imag() - in[pos+1].real() - in[pos+2].imag() + in[pos+3].real();
		yimag[1] = -in[pos+0].real() - in[pos+1].imag() + in[pos+2].real() + in[pos+3].imag();
		m_decimator2.myDecimate(xreal[0], yimag[0], &xreal[1], &yimag[1]);
        it->setReal(xreal[1] << norm_shift >> trunk_shift);
        it->setImag(yimag[1] << norm_shift >> trunk_shift);
        ++it;
	}

	sampleSize += (3 - trunk_shift);
}

/** double byte samples to double byte samples decimation by 8 centered */
void Decimators::decimate8_cen(unsigned int& sampleSize, const IQSampleVector& in, IQSampleVector& out)
{
	std::size_t len = in.size();
	out.resize(len/8);
	IQSampleVector::iterator it = out.begin();
	unsigned int trunk_shift = (sampleSize < 13 ? 0 : sampleSize - 13); // trunk to keep 16 bits (shift right)
	unsigned int norm_shift  = (sampleSize < 13 ? 13 - sampleSize : 0); // shift to normalize to 16 bits (shift left)
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

		it->setReal(intbuf[6] << norm_shift >> trunk_shift);
		it->setImag(intbuf[7] << norm_shift >> trunk_shift);
		++it;
	}

	sampleSize += (3 - trunk_shift);
}

/** double byte samples to double byte samples decimation by 16 low band */
void Decimators::decimate16_inf(unsigned int& sampleSize, const IQSampleVector& in, IQSampleVector& out)
{
	std::size_t len = in.size();
	out.resize(len/16);
	IQSampleVector::iterator it = out.begin();
	unsigned int trunk_shift = (sampleSize < 12 ? 0 : sampleSize - 12); // trunk to keep 16 bits (shift right)
	unsigned int norm_shift  = (sampleSize < 12 ? 12 - sampleSize : 0); // shift to normalize to 16 bits (shift left)

	std::int32_t xreal[4], yimag[4];

	for (unsigned int pos = 0; pos < len - 15; )
	{
		for (int i = 0; i < 4; i++)
		{
			xreal[i] = in[pos+0].real() - in[pos+1].imag() + in[pos+3].imag() - in[pos+2].real();
			yimag[i] = in[pos+0].imag() - in[pos+2].imag() + in[pos+1].real() - in[pos+3].real();
			pos += 4;
		}

		m_decimator2.myDecimate(xreal[0], yimag[0], &xreal[1], &yimag[1]);
		m_decimator2.myDecimate(xreal[2], yimag[2], &xreal[3], &yimag[3]);

		m_decimator4.myDecimate(xreal[1], yimag[1], &xreal[3], &yimag[3]);

		it->setReal(xreal[3] << norm_shift >> trunk_shift);
        it->setImag(yimag[3] << norm_shift >> trunk_shift);
        ++it;
	}

	sampleSize += (4 - trunk_shift);
}

/** double byte samples to double byte samples decimation by 16 high band */
void Decimators::decimate16_sup(unsigned int& sampleSize, const IQSampleVector& in, IQSampleVector& out)
{
	std::size_t len = in.size();
	out.resize(len/16);
	IQSampleVector::iterator it = out.begin();
	unsigned int trunk_shift = (sampleSize < 12 ? 0 : sampleSize - 12); // trunk to keep 16 bits (shift right)
	unsigned int norm_shift  = (sampleSize < 12 ? 12 - sampleSize : 0); // shift to normalize to 16 bits (shift left)

	std::int32_t xreal[4], yimag[4];

	for (unsigned int pos = 0; pos < len - 15; )
	{
		for (int i = 0; i < 4; i++)
		{
			xreal[i] =  in[pos+0].imag() - in[pos+1].real() - in[pos+2].imag() + in[pos+3].real();
			yimag[i] = -in[pos+0].real() - in[pos+1].imag() + in[pos+2].real() + in[pos+3].imag();
			pos += 4;
		}

		m_decimator2.myDecimate(xreal[0], yimag[0], &xreal[1], &yimag[1]);
		m_decimator2.myDecimate(xreal[2], yimag[2], &xreal[3], &yimag[3]);

		m_decimator4.myDecimate(xreal[1], yimag[1], &xreal[3], &yimag[3]);

		it->setReal(xreal[3] << norm_shift >> trunk_shift);
        it->setImag(yimag[3] << norm_shift >> trunk_shift);
        ++it;
	}

	sampleSize += (4 - trunk_shift);
}

/** double byte samples to double byte samples decimation by 16 centered */
void Decimators::decimate16_cen(unsigned int& sampleSize, const IQSampleVector& in, IQSampleVector& out)
{
	std::size_t len = in.size();
	out.resize(len/16);
	IQSampleVector::iterator it = out.begin();
	unsigned int trunk_shift = (sampleSize < 12 ? 0 : sampleSize - 12); // trunk to keep 16 bits (shift right)
	unsigned int norm_shift  = (sampleSize < 12 ? 12 - sampleSize : 0); // shift to normalize to 16 bits (shift left)
	int32_t intbuf[16];

	for (unsigned int pos = 0; pos < len - 15; pos += 16)
	{
		intbuf[0]  = in[pos+1].real();
		intbuf[1]  = in[pos+1].imag();
		intbuf[2]  = in[pos+3].real();
		intbuf[3]  = in[pos+3].imag();
		intbuf[4]  = in[pos+5].real();
		intbuf[5]  = in[pos+5].imag();
		intbuf[6]  = in[pos+7].real();
		intbuf[7]  = in[pos+7].imag();
		intbuf[8]  = in[pos+9].real();
		intbuf[9]  = in[pos+9].imag();
		intbuf[10] = in[pos+11].real();
		intbuf[11] = in[pos+11].imag();
		intbuf[12] = in[pos+13].real();
		intbuf[13] = in[pos+13].imag();
		intbuf[14] = in[pos+15].real();
		intbuf[15] = in[pos+15].imag();

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
		m_decimator2.myDecimate(
				in[pos+8].real(),
				in[pos+8].imag(),
				&intbuf[8],
				&intbuf[9]);
		m_decimator2.myDecimate(
				in[pos+10].real(),
				in[pos+10].imag(),
				&intbuf[10],
				&intbuf[11]);
		m_decimator2.myDecimate(
				in[pos+12].real(),
				in[pos+12].imag(),
				&intbuf[12],
				&intbuf[13]);
		m_decimator2.myDecimate(
				in[pos+14].real(),
				in[pos+14].imag(),
				&intbuf[14],
				&intbuf[15]);

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
		m_decimator4.myDecimate(
				intbuf[8],
				intbuf[9],
				&intbuf[10],
				&intbuf[11]);
		m_decimator4.myDecimate(
				intbuf[12],
				intbuf[13],
				&intbuf[14],
				&intbuf[15]);

		m_decimator8.myDecimate(
				intbuf[2],
				intbuf[3],
				&intbuf[6],
				&intbuf[7]);
		m_decimator8.myDecimate(
				intbuf[10],
				intbuf[11],
				&intbuf[14],
				&intbuf[15]);

		m_decimator16.myDecimate(
				intbuf[6],
				intbuf[7],
				&intbuf[14],
				&intbuf[15]);

		it->setReal(intbuf[14] << norm_shift >> trunk_shift);
		it->setImag(intbuf[15] << norm_shift >> trunk_shift);
		++it;
	}

	sampleSize += (4 - trunk_shift);
}

/** double byte samples to double byte samples decimation by 32 low band */
void Decimators::decimate32_inf(unsigned int& sampleSize, const IQSampleVector& in, IQSampleVector& out)
{
	std::size_t len = in.size();
	out.resize(len/32);
	IQSampleVector::iterator it = out.begin();
	unsigned int trunk_shift = (sampleSize < 11 ? 0 : sampleSize - 11); // trunk to keep 16 bits (shift right)
	unsigned int norm_shift  = (sampleSize < 11 ? 11 - sampleSize : 0); // shift to normalize to 16 bits (shift left)

	std::int32_t xreal[8], yimag[8];

	for (unsigned int pos = 0; pos < len - 31; )
	{
		for (int i = 0; i < 8; i++)
		{
			xreal[i] = in[pos+0].real() - in[pos+1].imag() + in[pos+3].imag() - in[pos+2].real();
			yimag[i] = in[pos+0].imag() - in[pos+2].imag() + in[pos+1].real() - in[pos+3].real();
			pos += 4;
		}

		m_decimator2.myDecimate(xreal[0], yimag[0], &xreal[1], &yimag[1]);
		m_decimator2.myDecimate(xreal[2], yimag[2], &xreal[3], &yimag[3]);
		m_decimator2.myDecimate(xreal[4], yimag[4], &xreal[5], &yimag[5]);
		m_decimator2.myDecimate(xreal[6], yimag[6], &xreal[7], &yimag[7]);

		m_decimator4.myDecimate(xreal[1], yimag[1], &xreal[3], &yimag[3]);
		m_decimator4.myDecimate(xreal[5], yimag[5], &xreal[7], &yimag[7]);

		m_decimator8.myDecimate(xreal[3], yimag[3], &xreal[7], &yimag[7]);

		it->setReal(xreal[7] << norm_shift >> trunk_shift);
        it->setImag(yimag[7] << norm_shift >> trunk_shift);
        ++it;
	}

	sampleSize += (5 - trunk_shift);
}

/** double byte samples to double byte samples decimation by 32 high band */
void Decimators::decimate32_sup(unsigned int& sampleSize, const IQSampleVector& in, IQSampleVector& out)
{
	std::size_t len = in.size();
	out.resize(len/32);
	IQSampleVector::iterator it = out.begin();
	unsigned int trunk_shift = (sampleSize < 11 ? 0 : sampleSize - 11); // trunk to keep 16 bits (shift right)
	unsigned int norm_shift  = (sampleSize < 11 ? 11 - sampleSize : 0); // shift to normalize to 16 bits (shift left)

	std::int32_t xreal[8], yimag[8];

	for (unsigned int pos = 0; pos < len - 31; )
	{
		for (int i = 0; i < 8; i++)
		{
			xreal[i] =  in[pos+0].imag() - in[pos+1].real() - in[pos+2].imag() + in[pos+3].real();
			yimag[i] = -in[pos+0].real() - in[pos+1].imag() + in[pos+2].real() + in[pos+3].imag();
			pos += 4;
		}

		m_decimator2.myDecimate(xreal[0], yimag[0], &xreal[1], &yimag[1]);
		m_decimator2.myDecimate(xreal[2], yimag[2], &xreal[3], &yimag[3]);
		m_decimator2.myDecimate(xreal[4], yimag[4], &xreal[5], &yimag[5]);
		m_decimator2.myDecimate(xreal[6], yimag[6], &xreal[7], &yimag[7]);

		m_decimator4.myDecimate(xreal[1], yimag[1], &xreal[3], &yimag[3]);
		m_decimator4.myDecimate(xreal[5], yimag[5], &xreal[7], &yimag[7]);

		m_decimator8.myDecimate(xreal[3], yimag[3], &xreal[7], &yimag[7]);

		it->setReal(xreal[7] << norm_shift >> trunk_shift);
        it->setImag(yimag[7] << norm_shift >> trunk_shift);
        ++it;
	}

	sampleSize += (5 - trunk_shift);
}

/** double byte samples to double byte samples decimation by 32 centered */
void Decimators::decimate32_cen(unsigned int& sampleSize, const IQSampleVector& in, IQSampleVector& out)
{
	std::size_t len = in.size();
	out.resize(len/32);
	IQSampleVector::iterator it = out.begin();
	unsigned int trunk_shift = (sampleSize < 11 ? 0 : sampleSize - 11); // trunk to keep 16 bits (shift right)
	unsigned int norm_shift  = (sampleSize < 11 ? 11 - sampleSize : 0); // shift to normalize to 16 bits (shift left)
	int32_t intbuf[32];

	for (unsigned int pos = 0; pos < len - 31; pos += 32)
	{
		intbuf[0]  = in[pos+1].real();
		intbuf[1]  = in[pos+1].imag();
		intbuf[2]  = in[pos+3].real();
		intbuf[3]  = in[pos+3].imag();
		intbuf[4]  = in[pos+5].real();
		intbuf[5]  = in[pos+5].imag();
		intbuf[6]  = in[pos+7].real();
		intbuf[7]  = in[pos+7].imag();
		intbuf[8]  = in[pos+9].real();
		intbuf[9]  = in[pos+9].imag();
		intbuf[10] = in[pos+11].real();
		intbuf[11] = in[pos+11].imag();
		intbuf[12] = in[pos+13].real();
		intbuf[13] = in[pos+13].imag();
		intbuf[14] = in[pos+15].real();
		intbuf[15] = in[pos+15].imag();
		intbuf[16] = in[pos+17].real();
		intbuf[17] = in[pos+17].imag();
		intbuf[18] = in[pos+19].real();
		intbuf[19] = in[pos+19].imag();
		intbuf[20] = in[pos+21].real();
		intbuf[21] = in[pos+21].imag();
		intbuf[22] = in[pos+23].real();
		intbuf[23] = in[pos+23].imag();
		intbuf[24] = in[pos+25].real();
		intbuf[25] = in[pos+25].imag();
		intbuf[26] = in[pos+27].real();
		intbuf[27] = in[pos+27].imag();
		intbuf[28] = in[pos+29].real();
		intbuf[29] = in[pos+29].imag();
		intbuf[30] = in[pos+31].real();
		intbuf[31] = in[pos+31].imag();

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
		m_decimator2.myDecimate(
				in[pos+8].real(),
				in[pos+8].imag(),
				&intbuf[8],
				&intbuf[9]);
		m_decimator2.myDecimate(
				in[pos+10].real(),
				in[pos+10].imag(),
				&intbuf[10],
				&intbuf[11]);
		m_decimator2.myDecimate(
				in[pos+12].real(),
				in[pos+12].imag(),
				&intbuf[12],
				&intbuf[13]);
		m_decimator2.myDecimate(
				in[pos+14].real(),
				in[pos+14].imag(),
				&intbuf[14],
				&intbuf[15]);
		m_decimator2.myDecimate(
				in[pos+16].real(),
				in[pos+16].imag(),
				&intbuf[16],
				&intbuf[17]);
		m_decimator2.myDecimate(
				in[pos+18].real(),
				in[pos+18].imag(),
				&intbuf[18],
				&intbuf[19]);
		m_decimator2.myDecimate(
				in[pos+20].real(),
				in[pos+20].imag(),
				&intbuf[20],
				&intbuf[21]);
		m_decimator2.myDecimate(
				in[pos+22].real(),
				in[pos+22].imag(),
				&intbuf[22],
				&intbuf[23]);
		m_decimator2.myDecimate(
				in[pos+24].real(),
				in[pos+24].imag(),
				&intbuf[24],
				&intbuf[25]);
		m_decimator2.myDecimate(
				in[pos+26].real(),
				in[pos+26].imag(),
				&intbuf[26],
				&intbuf[27]);
		m_decimator2.myDecimate(
				in[pos+28].real(),
				in[pos+28].imag(),
				&intbuf[28],
				&intbuf[29]);
		m_decimator2.myDecimate(
				in[pos+30].real(),
				in[pos+30].imag(),
				&intbuf[30],
				&intbuf[31]);

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
		m_decimator4.myDecimate(
				intbuf[8],
				intbuf[9],
				&intbuf[10],
				&intbuf[11]);
		m_decimator4.myDecimate(
				intbuf[12],
				intbuf[13],
				&intbuf[14],
				&intbuf[15]);
		m_decimator4.myDecimate(
				intbuf[16],
				intbuf[17],
				&intbuf[18],
				&intbuf[19]);
		m_decimator4.myDecimate(
				intbuf[20],
				intbuf[21],
				&intbuf[22],
				&intbuf[23]);
		m_decimator4.myDecimate(
				intbuf[24],
				intbuf[25],
				&intbuf[26],
				&intbuf[27]);
		m_decimator4.myDecimate(
				intbuf[28],
				intbuf[29],
				&intbuf[30],
				&intbuf[31]);

		m_decimator8.myDecimate(
				intbuf[2],
				intbuf[3],
				&intbuf[6],
				&intbuf[7]);
		m_decimator8.myDecimate(
				intbuf[10],
				intbuf[11],
				&intbuf[14],
				&intbuf[15]);
		m_decimator8.myDecimate(
				intbuf[18],
				intbuf[19],
				&intbuf[22],
				&intbuf[23]);
		m_decimator8.myDecimate(
				intbuf[26],
				intbuf[27],
				&intbuf[30],
				&intbuf[31]);

		m_decimator16.myDecimate(
				intbuf[6],
				intbuf[7],
				&intbuf[14],
				&intbuf[15]);
		m_decimator16.myDecimate(
				intbuf[22],
				intbuf[23],
				&intbuf[30],
				&intbuf[31]);

		m_decimator32.myDecimate(
				intbuf[14],
				intbuf[15],
				&intbuf[30],
				&intbuf[31]);

		it->setReal(intbuf[30] << norm_shift >> trunk_shift);
		it->setImag(intbuf[31] << norm_shift >> trunk_shift);
		++it;
	}

	sampleSize += (5 - trunk_shift);
}

/** double byte samples to double byte samples decimation by 64 low band */
void Decimators::decimate64_inf(unsigned int& sampleSize, const IQSampleVector& in, IQSampleVector& out)
{
	std::size_t len = in.size();
	out.resize(len/64);
	IQSampleVector::iterator it = out.begin();
	unsigned int trunk_shift = (sampleSize < 10 ? 0 : sampleSize - 10); // trunk to keep 16 bits (shift right)
	unsigned int norm_shift  = (sampleSize < 10 ? 10 - sampleSize : 0); // shift to normalize to 16 bits (shift left)

	std::int32_t xreal[16], yimag[16];

	for (unsigned int pos = 0; pos < len - 63; )
	{
		for (int i = 0; i < 16; i++)
		{
			xreal[i] = in[pos+0].real() - in[pos+1].imag() + in[pos+3].imag() - in[pos+2].real();
			yimag[i] = in[pos+0].imag() - in[pos+2].imag() + in[pos+1].real() - in[pos+3].real();
			pos += 4;
		}

		m_decimator2.myDecimate(xreal[0], yimag[0], &xreal[1], &yimag[1]);
		m_decimator2.myDecimate(xreal[2], yimag[2], &xreal[3], &yimag[3]);
		m_decimator2.myDecimate(xreal[4], yimag[4], &xreal[5], &yimag[5]);
		m_decimator2.myDecimate(xreal[6], yimag[6], &xreal[7], &yimag[7]);
		m_decimator2.myDecimate(xreal[8], yimag[8], &xreal[9], &yimag[9]);
		m_decimator2.myDecimate(xreal[10], yimag[10], &xreal[11], &yimag[11]);
		m_decimator2.myDecimate(xreal[12], yimag[12], &xreal[13], &yimag[13]);
		m_decimator2.myDecimate(xreal[14], yimag[14], &xreal[15], &yimag[15]);

		m_decimator4.myDecimate(xreal[1], yimag[1], &xreal[3], &yimag[3]);
		m_decimator4.myDecimate(xreal[5], yimag[5], &xreal[7], &yimag[7]);
		m_decimator4.myDecimate(xreal[9], yimag[9], &xreal[11], &yimag[11]);
		m_decimator4.myDecimate(xreal[13], yimag[13], &xreal[15], &yimag[15]);

		m_decimator8.myDecimate(xreal[3], yimag[3], &xreal[7], &yimag[7]);
		m_decimator8.myDecimate(xreal[11], yimag[11], &xreal[15], &yimag[15]);

		m_decimator16.myDecimate(xreal[7], yimag[7], &xreal[15], &yimag[15]);

		it->setReal(xreal[15] << norm_shift >> trunk_shift);
        it->setImag(yimag[15] << norm_shift >> trunk_shift);
        ++it;
	}

	sampleSize += (6 - trunk_shift);
}

/** double byte samples to double byte samples decimation by 64 high band */
void Decimators::decimate64_sup(unsigned int& sampleSize, const IQSampleVector& in, IQSampleVector& out)
{
	std::size_t len = in.size();
	out.resize(len/64);
	IQSampleVector::iterator it = out.begin();
	unsigned int trunk_shift = (sampleSize < 10 ? 0 : sampleSize - 10); // trunk to keep 16 bits (shift right)
	unsigned int norm_shift  = (sampleSize < 10 ? 10 - sampleSize : 0); // shift to normalize to 16 bits (shift left)

	std::int32_t xreal[16], yimag[16];

	for (unsigned int pos = 0; pos < len - 63; )
	{
		for (int i = 0; i < 16; i++)
		{
			xreal[i] =  in[pos+0].imag() - in[pos+1].real() - in[pos+2].imag() + in[pos+3].real();
			yimag[i] = -in[pos+0].real() - in[pos+1].imag() + in[pos+2].real() + in[pos+3].imag();
			pos += 4;
		}

		m_decimator2.myDecimate(xreal[0], yimag[0], &xreal[1], &yimag[1]);
		m_decimator2.myDecimate(xreal[2], yimag[2], &xreal[3], &yimag[3]);
		m_decimator2.myDecimate(xreal[4], yimag[4], &xreal[5], &yimag[5]);
		m_decimator2.myDecimate(xreal[6], yimag[6], &xreal[7], &yimag[7]);
		m_decimator2.myDecimate(xreal[8], yimag[8], &xreal[9], &yimag[9]);
		m_decimator2.myDecimate(xreal[10], yimag[10], &xreal[11], &yimag[11]);
		m_decimator2.myDecimate(xreal[12], yimag[12], &xreal[13], &yimag[13]);
		m_decimator2.myDecimate(xreal[14], yimag[14], &xreal[15], &yimag[15]);

		m_decimator4.myDecimate(xreal[1], yimag[1], &xreal[3], &yimag[3]);
		m_decimator4.myDecimate(xreal[5], yimag[5], &xreal[7], &yimag[7]);
		m_decimator4.myDecimate(xreal[9], yimag[9], &xreal[11], &yimag[11]);
		m_decimator4.myDecimate(xreal[13], yimag[13], &xreal[15], &yimag[15]);

		m_decimator8.myDecimate(xreal[3], yimag[3], &xreal[7], &yimag[7]);
		m_decimator8.myDecimate(xreal[11], yimag[11], &xreal[15], &yimag[15]);

		m_decimator16.myDecimate(xreal[7], yimag[7], &xreal[15], &yimag[15]);

		it->setReal(xreal[15] << norm_shift >> trunk_shift);
        it->setImag(yimag[15] << norm_shift >> trunk_shift);
        ++it;
	}

	sampleSize += (6 - trunk_shift);
}

/** double byte samples to double byte samples decimation by 64 centered */
void Decimators::decimate64_cen(unsigned int& sampleSize, const IQSampleVector& in, IQSampleVector& out)
{
	std::size_t len = in.size();
	out.resize(len/64);
	IQSampleVector::iterator it = out.begin();
	unsigned int trunk_shift = (sampleSize < 10 ? 0 : sampleSize - 10); // trunk to keep 16 bits (shift right)
	unsigned int norm_shift  = (sampleSize < 10 ? 10 - sampleSize : 0); // shift to normalize to 16 bits (shift left)
	int32_t intbuf[64];

	for (unsigned int pos = 0; pos < len - 63; pos += 64)
	{
		intbuf[0]  = in[pos+1].real();
		intbuf[1]  = in[pos+1].imag();
		intbuf[2]  = in[pos+3].real();
		intbuf[3]  = in[pos+3].imag();
		intbuf[4]  = in[pos+5].real();
		intbuf[5]  = in[pos+5].imag();
		intbuf[6]  = in[pos+7].real();
		intbuf[7]  = in[pos+7].imag();
		intbuf[8]  = in[pos+9].real();
		intbuf[9]  = in[pos+9].imag();
		intbuf[10] = in[pos+11].real();
		intbuf[11] = in[pos+11].imag();
		intbuf[12] = in[pos+13].real();
		intbuf[13] = in[pos+13].imag();
		intbuf[14] = in[pos+15].real();
		intbuf[15] = in[pos+15].imag();
		intbuf[16] = in[pos+17].real();
		intbuf[17] = in[pos+17].imag();
		intbuf[18] = in[pos+19].real();
		intbuf[19] = in[pos+19].imag();
		intbuf[20] = in[pos+21].real();
		intbuf[21] = in[pos+21].imag();
		intbuf[22] = in[pos+23].real();
		intbuf[23] = in[pos+23].imag();
		intbuf[24] = in[pos+25].real();
		intbuf[25] = in[pos+25].imag();
		intbuf[26] = in[pos+27].real();
		intbuf[27] = in[pos+27].imag();
		intbuf[28] = in[pos+29].real();
		intbuf[29] = in[pos+29].imag();
		intbuf[30] = in[pos+31].real();
		intbuf[31] = in[pos+31].imag();
		intbuf[32] = in[pos+33].real();
		intbuf[33] = in[pos+33].imag();
		intbuf[34] = in[pos+35].real();
		intbuf[35] = in[pos+35].imag();
		intbuf[36] = in[pos+37].real();
		intbuf[37] = in[pos+37].imag();
		intbuf[38] = in[pos+39].real();
		intbuf[39] = in[pos+39].imag();
		intbuf[40] = in[pos+41].real();
		intbuf[41] = in[pos+41].imag();
		intbuf[42] = in[pos+43].real();
		intbuf[43] = in[pos+43].imag();
		intbuf[44] = in[pos+45].real();
		intbuf[45] = in[pos+45].imag();
		intbuf[46] = in[pos+47].real();
		intbuf[47] = in[pos+47].imag();
		intbuf[48] = in[pos+49].real();
		intbuf[49] = in[pos+49].imag();
		intbuf[50] = in[pos+51].real();
		intbuf[51] = in[pos+51].imag();
		intbuf[52] = in[pos+53].real();
		intbuf[53] = in[pos+53].imag();
		intbuf[54] = in[pos+55].real();
		intbuf[55] = in[pos+55].imag();
		intbuf[56] = in[pos+57].real();
		intbuf[57] = in[pos+57].imag();
		intbuf[58] = in[pos+59].real();
		intbuf[59] = in[pos+59].imag();
		intbuf[60] = in[pos+61].real();
		intbuf[61] = in[pos+61].imag();
		intbuf[62] = in[pos+63].real();
		intbuf[63] = in[pos+63].imag();

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
		m_decimator2.myDecimate(
				in[pos+8].real(),
				in[pos+8].imag(),
				&intbuf[8],
				&intbuf[9]);
		m_decimator2.myDecimate(
				in[pos+10].real(),
				in[pos+10].imag(),
				&intbuf[10],
				&intbuf[11]);
		m_decimator2.myDecimate(
				in[pos+12].real(),
				in[pos+12].imag(),
				&intbuf[12],
				&intbuf[13]);
		m_decimator2.myDecimate(
				in[pos+14].real(),
				in[pos+14].imag(),
				&intbuf[14],
				&intbuf[15]);
		m_decimator2.myDecimate(
				in[pos+16].real(),
				in[pos+16].imag(),
				&intbuf[16],
				&intbuf[17]);
		m_decimator2.myDecimate(
				in[pos+18].real(),
				in[pos+18].imag(),
				&intbuf[18],
				&intbuf[19]);
		m_decimator2.myDecimate(
				in[pos+20].real(),
				in[pos+20].imag(),
				&intbuf[20],
				&intbuf[21]);
		m_decimator2.myDecimate(
				in[pos+22].real(),
				in[pos+22].imag(),
				&intbuf[22],
				&intbuf[23]);
		m_decimator2.myDecimate(
				in[pos+24].real(),
				in[pos+24].imag(),
				&intbuf[24],
				&intbuf[25]);
		m_decimator2.myDecimate(
				in[pos+26].real(),
				in[pos+26].imag(),
				&intbuf[26],
				&intbuf[27]);
		m_decimator2.myDecimate(
				in[pos+28].real(),
				in[pos+28].imag(),
				&intbuf[28],
				&intbuf[29]);
		m_decimator2.myDecimate(
				in[pos+30].real(),
				in[pos+30].imag(),
				&intbuf[30],
				&intbuf[31]);
		m_decimator2.myDecimate(
				in[pos+32].real(),
				in[pos+32].imag(),
				&intbuf[32],
				&intbuf[33]);
		m_decimator2.myDecimate(
				in[pos+34].real(),
				in[pos+34].imag(),
				&intbuf[34],
				&intbuf[35]);
		m_decimator2.myDecimate(
				in[pos+36].real(),
				in[pos+36].imag(),
				&intbuf[36],
				&intbuf[37]);
		m_decimator2.myDecimate(
				in[pos+38].real(),
				in[pos+38].imag(),
				&intbuf[38],
				&intbuf[39]);
		m_decimator2.myDecimate(
				in[pos+40].real(),
				in[pos+40].imag(),
				&intbuf[40],
				&intbuf[41]);
		m_decimator2.myDecimate(
				in[pos+42].real(),
				in[pos+42].imag(),
				&intbuf[42],
				&intbuf[43]);
		m_decimator2.myDecimate(
				in[pos+44].real(),
				in[pos+44].imag(),
				&intbuf[44],
				&intbuf[45]);
		m_decimator2.myDecimate(
				in[pos+46].real(),
				in[pos+46].imag(),
				&intbuf[46],
				&intbuf[47]);
		m_decimator2.myDecimate(
				in[pos+48].real(),
				in[pos+48].imag(),
				&intbuf[48],
				&intbuf[49]);
		m_decimator2.myDecimate(
				in[pos+50].real(),
				in[pos+50].imag(),
				&intbuf[50],
				&intbuf[51]);
		m_decimator2.myDecimate(
				in[pos+52].real(),
				in[pos+52].imag(),
				&intbuf[52],
				&intbuf[53]);
		m_decimator2.myDecimate(
				in[pos+54].real(),
				in[pos+54].imag(),
				&intbuf[54],
				&intbuf[55]);
		m_decimator2.myDecimate(
				in[pos+56].real(),
				in[pos+56].imag(),
				&intbuf[56],
				&intbuf[57]);
		m_decimator2.myDecimate(
				in[pos+58].real(),
				in[pos+58].imag(),
				&intbuf[58],
				&intbuf[59]);
		m_decimator2.myDecimate(
				in[pos+60].real(),
				in[pos+60].imag(),
				&intbuf[60],
				&intbuf[61]);
		m_decimator2.myDecimate(
				in[pos+62].real(),
				in[pos+62].imag(),
				&intbuf[62],
				&intbuf[63]);

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
		m_decimator4.myDecimate(
				intbuf[8],
				intbuf[9],
				&intbuf[10],
				&intbuf[11]);
		m_decimator4.myDecimate(
				intbuf[12],
				intbuf[13],
				&intbuf[14],
				&intbuf[15]);
		m_decimator4.myDecimate(
				intbuf[16],
				intbuf[17],
				&intbuf[18],
				&intbuf[19]);
		m_decimator4.myDecimate(
				intbuf[20],
				intbuf[21],
				&intbuf[22],
				&intbuf[23]);
		m_decimator4.myDecimate(
				intbuf[24],
				intbuf[25],
				&intbuf[26],
				&intbuf[27]);
		m_decimator4.myDecimate(
				intbuf[28],
				intbuf[29],
				&intbuf[30],
				&intbuf[31]);
		m_decimator4.myDecimate(
				intbuf[32],
				intbuf[33],
				&intbuf[34],
				&intbuf[35]);
		m_decimator4.myDecimate(
				intbuf[36],
				intbuf[37],
				&intbuf[38],
				&intbuf[39]);
		m_decimator4.myDecimate(
				intbuf[40],
				intbuf[41],
				&intbuf[42],
				&intbuf[43]);
		m_decimator4.myDecimate(
				intbuf[44],
				intbuf[45],
				&intbuf[46],
				&intbuf[47]);
		m_decimator4.myDecimate(
				intbuf[48],
				intbuf[49],
				&intbuf[50],
				&intbuf[51]);
		m_decimator4.myDecimate(
				intbuf[52],
				intbuf[53],
				&intbuf[54],
				&intbuf[55]);
		m_decimator4.myDecimate(
				intbuf[56],
				intbuf[57],
				&intbuf[58],
				&intbuf[59]);
		m_decimator4.myDecimate(
				intbuf[60],
				intbuf[61],
				&intbuf[62],
				&intbuf[63]);

		m_decimator8.myDecimate(
				intbuf[2],
				intbuf[3],
				&intbuf[6],
				&intbuf[7]);
		m_decimator8.myDecimate(
				intbuf[10],
				intbuf[11],
				&intbuf[14],
				&intbuf[15]);
		m_decimator8.myDecimate(
				intbuf[18],
				intbuf[19],
				&intbuf[22],
				&intbuf[23]);
		m_decimator8.myDecimate(
				intbuf[26],
				intbuf[27],
				&intbuf[30],
				&intbuf[31]);
		m_decimator8.myDecimate(
				intbuf[34],
				intbuf[35],
				&intbuf[38],
				&intbuf[39]);
		m_decimator8.myDecimate(
				intbuf[42],
				intbuf[43],
				&intbuf[46],
				&intbuf[47]);
		m_decimator8.myDecimate(
				intbuf[50],
				intbuf[51],
				&intbuf[54],
				&intbuf[55]);
		m_decimator8.myDecimate(
				intbuf[58],
				intbuf[59],
				&intbuf[62],
				&intbuf[63]);

		m_decimator16.myDecimate(
				intbuf[6],
				intbuf[7],
				&intbuf[14],
				&intbuf[15]);
		m_decimator16.myDecimate(
				intbuf[22],
				intbuf[23],
				&intbuf[30],
				&intbuf[31]);
		m_decimator16.myDecimate(
				intbuf[38],
				intbuf[39],
				&intbuf[46],
				&intbuf[47]);
		m_decimator16.myDecimate(
				intbuf[54],
				intbuf[55],
				&intbuf[62],
				&intbuf[63]);

		m_decimator32.myDecimate(
				intbuf[14],
				intbuf[15],
				&intbuf[30],
				&intbuf[31]);
		m_decimator32.myDecimate(
				intbuf[46],
				intbuf[47],
				&intbuf[62],
				&intbuf[63]);

		m_decimator64.myDecimate(
				intbuf[30],
				intbuf[31],
				&intbuf[62],
				&intbuf[63]);

		it->setReal(intbuf[62] << norm_shift >> trunk_shift);
		it->setImag(intbuf[63] << norm_shift >> trunk_shift);
		++it;
	}

	sampleSize += (6 - trunk_shift);
}
