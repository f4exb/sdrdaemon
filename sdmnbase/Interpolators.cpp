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

#include <string.h>
#include "Interpolators.h"

void Interpolators::interpolate2_cen(unsigned int deviceSampleSize, const IQSampleVector& in, IQSampleVector& out)
{
    int len = in.size();
    out.resize(len*2);
    IQSampleVector::const_iterator itIn = in.begin();
    IQSampleVector::iterator itOut = out.begin();
    unsigned int trunk_shift  = (deviceSampleSize > 16 ? 0 : 16 - deviceSampleSize); // shift to normalize to number of output bits (shift left)
    int32_t intbuf[4];

    for (; itIn != in.end(); ++itIn)
    {
        intbuf[0]  = itIn->real();
        intbuf[1]  = itIn->imag();

        m_interpolator2.myInterpolate(&intbuf[0], &intbuf[1], &intbuf[2], &intbuf[3]);

        itOut->setReal(intbuf[0] >> trunk_shift);
        itOut->setImag(intbuf[1] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[2] >> trunk_shift);
        itOut->setImag(intbuf[3] >> trunk_shift);
        ++itOut;
    }
}

void Interpolators::interpolate4_cen(unsigned int deviceSampleSize, const IQSampleVector& in, IQSampleVector& out)
{
    int len = in.size();
    out.resize(len*4);
    IQSampleVector::const_iterator itIn = in.begin();
    IQSampleVector::iterator itOut = out.begin();
    unsigned int trunk_shift  = (deviceSampleSize > 16 ? 0 : 16 - deviceSampleSize); // shift to normalize to number of output bits (shift left)
    int32_t intbuf[8];

    for (; itIn != in.end(); ++itIn)
    {
        intbuf[0]  = itIn->real();
        intbuf[1]  = itIn->imag();

        m_interpolator2.myInterpolate(&intbuf[0], &intbuf[1], &intbuf[4], &intbuf[5]);

        m_interpolator4.myInterpolate(&intbuf[0], &intbuf[1], &intbuf[2], &intbuf[3]);
        m_interpolator4.myInterpolate(&intbuf[4], &intbuf[5], &intbuf[6], &intbuf[7]);

        itOut->setReal(intbuf[0] >> trunk_shift);
        itOut->setImag(intbuf[1] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[2] >> trunk_shift);
        itOut->setImag(intbuf[3] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[4] >> trunk_shift);
        itOut->setImag(intbuf[5] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[6] >> trunk_shift);
        itOut->setImag(intbuf[7] >> trunk_shift);
        ++itOut;
    }
}

void Interpolators::interpolate8_cen(unsigned int deviceSampleSize, const IQSampleVector& in, IQSampleVector& out)
{
    int len = in.size();
    out.resize(len*8);
    IQSampleVector::const_iterator itIn = in.begin();
    IQSampleVector::iterator itOut = out.begin();
    unsigned int trunk_shift  = (deviceSampleSize > 16 ? 0 : 16 - deviceSampleSize); // shift to normalize to number of output bits (shift left)
    int32_t intbuf[16];

    for (; itIn != in.end(); ++itIn)
    {
        intbuf[0]  = itIn->real();
        intbuf[1]  = itIn->imag();

        m_interpolator2.myInterpolate(&intbuf[0], &intbuf[1], &intbuf[8], &intbuf[9]);

        m_interpolator4.myInterpolate(&intbuf[0], &intbuf[1], &intbuf[4], &intbuf[5]);
        m_interpolator4.myInterpolate(&intbuf[8], &intbuf[9], &intbuf[12], &intbuf[13]);

        m_interpolator8.myInterpolate(&intbuf[0], &intbuf[1], &intbuf[2], &intbuf[3]);
        m_interpolator8.myInterpolate(&intbuf[4], &intbuf[5], &intbuf[6], &intbuf[7]);
        m_interpolator8.myInterpolate(&intbuf[8], &intbuf[9], &intbuf[10], &intbuf[11]);
        m_interpolator8.myInterpolate(&intbuf[12], &intbuf[13], &intbuf[14], &intbuf[15]);

        itOut->setReal(intbuf[0] >> trunk_shift);
        itOut->setImag(intbuf[1] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[2] >> trunk_shift);
        itOut->setImag(intbuf[3] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[4] >> trunk_shift);
        itOut->setImag(intbuf[5] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[6] >> trunk_shift);
        itOut->setImag(intbuf[7] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[8] >> trunk_shift);
        itOut->setImag(intbuf[9] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[10] >> trunk_shift);
        itOut->setImag(intbuf[11] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[12] >> trunk_shift);
        itOut->setImag(intbuf[13] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[14] >> trunk_shift);
        itOut->setImag(intbuf[15] >> trunk_shift);
        ++itOut;
    }
}

void Interpolators::interpolate16_cen(unsigned int deviceSampleSize, const IQSampleVector& in, IQSampleVector& out)
{
    int len = in.size();
    out.resize(len*16);
    IQSampleVector::const_iterator itIn = in.begin();
    IQSampleVector::iterator itOut = out.begin();
    unsigned int trunk_shift  = (deviceSampleSize > 16 ? 0 : 16 - deviceSampleSize); // shift to normalize to number of output bits (shift left)
    int32_t intbuf[32];

    for (; itIn != in.end(); ++itIn)
    {
        intbuf[0]  = itIn->real();
        intbuf[1]  = itIn->imag();

        m_interpolator2.myInterpolate(&intbuf[0], &intbuf[1], &intbuf[16], &intbuf[17]);

        m_interpolator4.myInterpolate(&intbuf[0], &intbuf[1], &intbuf[8], &intbuf[9]);
        m_interpolator4.myInterpolate(&intbuf[16], &intbuf[17], &intbuf[24], &intbuf[25]);

        m_interpolator8.myInterpolate(&intbuf[0], &intbuf[1], &intbuf[4], &intbuf[5]);
        m_interpolator8.myInterpolate(&intbuf[8], &intbuf[9], &intbuf[12], &intbuf[13]);
        m_interpolator8.myInterpolate(&intbuf[16], &intbuf[17], &intbuf[20], &intbuf[21]);
        m_interpolator8.myInterpolate(&intbuf[24], &intbuf[25], &intbuf[28], &intbuf[29]);

        m_interpolator16.myInterpolate(&intbuf[0], &intbuf[1], &intbuf[2], &intbuf[3]);
        m_interpolator16.myInterpolate(&intbuf[4], &intbuf[5], &intbuf[6], &intbuf[7]);
        m_interpolator16.myInterpolate(&intbuf[8], &intbuf[9], &intbuf[10], &intbuf[11]);
        m_interpolator16.myInterpolate(&intbuf[12], &intbuf[13], &intbuf[14], &intbuf[15]);
        m_interpolator16.myInterpolate(&intbuf[16], &intbuf[17], &intbuf[18], &intbuf[19]);
        m_interpolator16.myInterpolate(&intbuf[20], &intbuf[21], &intbuf[22], &intbuf[23]);
        m_interpolator16.myInterpolate(&intbuf[24], &intbuf[25], &intbuf[26], &intbuf[27]);
        m_interpolator16.myInterpolate(&intbuf[28], &intbuf[29], &intbuf[30], &intbuf[31]);

        itOut->setReal(intbuf[0] >> trunk_shift);
        itOut->setImag(intbuf[1] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[2] >> trunk_shift);
        itOut->setImag(intbuf[3] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[4] >> trunk_shift);
        itOut->setImag(intbuf[5] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[6] >> trunk_shift);
        itOut->setImag(intbuf[7] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[8] >> trunk_shift);
        itOut->setImag(intbuf[9] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[10] >> trunk_shift);
        itOut->setImag(intbuf[11] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[12] >> trunk_shift);
        itOut->setImag(intbuf[13] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[14] >> trunk_shift);
        itOut->setImag(intbuf[15] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[16] >> trunk_shift);
        itOut->setImag(intbuf[17] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[18] >> trunk_shift);
        itOut->setImag(intbuf[19] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[20] >> trunk_shift);
        itOut->setImag(intbuf[21] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[22] >> trunk_shift);
        itOut->setImag(intbuf[23] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[24] >> trunk_shift);
        itOut->setImag(intbuf[25] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[26] >> trunk_shift);
        itOut->setImag(intbuf[27] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[28] >> trunk_shift);
        itOut->setImag(intbuf[29] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[30] >> trunk_shift);
        itOut->setImag(intbuf[31] >> trunk_shift);
        ++itOut;
    }
}

void Interpolators::interpolate32_cen(unsigned int deviceSampleSize, const IQSampleVector& in, IQSampleVector& out)
{
    int len = in.size();
    out.resize(len*32);
    IQSampleVector::const_iterator itIn = in.begin();
    IQSampleVector::iterator itOut = out.begin();
    unsigned int trunk_shift  = (deviceSampleSize > 16 ? 0 : 16 - deviceSampleSize); // shift to normalize to number of output bits (shift left)
    int32_t intbuf[64];

    for (; itIn != in.end(); ++itIn)
    {
        intbuf[0]  = itIn->real();
        intbuf[1]  = itIn->imag();

        m_interpolator2.myInterpolate(&intbuf[0], &intbuf[1], &intbuf[32], &intbuf[33]);


        m_interpolator4.myInterpolate(&intbuf[0],  &intbuf[1],  &intbuf[16], &intbuf[17]);
        m_interpolator4.myInterpolate(&intbuf[32], &intbuf[33], &intbuf[48], &intbuf[49]);


        m_interpolator8.myInterpolate(&intbuf[0],  &intbuf[1],  &intbuf[8],  &intbuf[9]);
        m_interpolator8.myInterpolate(&intbuf[16], &intbuf[17], &intbuf[24], &intbuf[25]);
        m_interpolator8.myInterpolate(&intbuf[32], &intbuf[33], &intbuf[40], &intbuf[41]);
        m_interpolator8.myInterpolate(&intbuf[48], &intbuf[49], &intbuf[56], &intbuf[57]);

        m_interpolator16.myInterpolate(&intbuf[0],  &intbuf[1],  &intbuf[4],  &intbuf[5]);
        m_interpolator16.myInterpolate(&intbuf[8],  &intbuf[9],  &intbuf[12], &intbuf[13]);
        m_interpolator16.myInterpolate(&intbuf[16], &intbuf[17], &intbuf[20], &intbuf[21]);
        m_interpolator16.myInterpolate(&intbuf[24], &intbuf[25], &intbuf[28], &intbuf[29]);
        m_interpolator16.myInterpolate(&intbuf[32], &intbuf[33], &intbuf[36], &intbuf[37]);
        m_interpolator16.myInterpolate(&intbuf[40], &intbuf[41], &intbuf[44], &intbuf[45]);
        m_interpolator16.myInterpolate(&intbuf[48], &intbuf[49], &intbuf[52], &intbuf[53]);
        m_interpolator16.myInterpolate(&intbuf[56], &intbuf[57], &intbuf[60], &intbuf[61]);

        m_interpolator32.myInterpolate(&intbuf[0],  &intbuf[1],  &intbuf[2],  &intbuf[3]);
        m_interpolator32.myInterpolate(&intbuf[4],  &intbuf[5],  &intbuf[6],  &intbuf[7]);
        m_interpolator32.myInterpolate(&intbuf[8],  &intbuf[9],  &intbuf[10], &intbuf[11]);
        m_interpolator32.myInterpolate(&intbuf[12], &intbuf[13], &intbuf[14], &intbuf[15]);
        m_interpolator32.myInterpolate(&intbuf[16], &intbuf[17], &intbuf[18], &intbuf[19]);
        m_interpolator32.myInterpolate(&intbuf[20], &intbuf[21], &intbuf[22], &intbuf[23]);
        m_interpolator32.myInterpolate(&intbuf[24], &intbuf[25], &intbuf[26], &intbuf[27]);
        m_interpolator32.myInterpolate(&intbuf[28], &intbuf[29], &intbuf[30], &intbuf[31]);
        m_interpolator32.myInterpolate(&intbuf[32], &intbuf[33], &intbuf[34], &intbuf[35]);
        m_interpolator32.myInterpolate(&intbuf[36], &intbuf[37], &intbuf[38], &intbuf[39]);
        m_interpolator32.myInterpolate(&intbuf[40], &intbuf[41], &intbuf[42], &intbuf[43]);
        m_interpolator32.myInterpolate(&intbuf[44], &intbuf[45], &intbuf[46], &intbuf[47]);
        m_interpolator32.myInterpolate(&intbuf[48], &intbuf[49], &intbuf[50], &intbuf[51]);
        m_interpolator32.myInterpolate(&intbuf[52], &intbuf[53], &intbuf[54], &intbuf[55]);
        m_interpolator32.myInterpolate(&intbuf[56], &intbuf[57], &intbuf[58], &intbuf[59]);
        m_interpolator32.myInterpolate(&intbuf[60], &intbuf[61], &intbuf[62], &intbuf[63]);

        itOut->setReal(intbuf[0] >> trunk_shift);
        itOut->setImag(intbuf[1] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[2] >> trunk_shift);
        itOut->setImag(intbuf[3] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[4] >> trunk_shift);
        itOut->setImag(intbuf[5] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[6] >> trunk_shift);
        itOut->setImag(intbuf[7] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[8] >> trunk_shift);
        itOut->setImag(intbuf[9] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[10] >> trunk_shift);
        itOut->setImag(intbuf[11] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[12] >> trunk_shift);
        itOut->setImag(intbuf[13] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[14] >> trunk_shift);
        itOut->setImag(intbuf[15] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[16] >> trunk_shift);
        itOut->setImag(intbuf[17] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[18] >> trunk_shift);
        itOut->setImag(intbuf[19] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[20] >> trunk_shift);
        itOut->setImag(intbuf[21] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[22] >> trunk_shift);
        itOut->setImag(intbuf[23] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[24] >> trunk_shift);
        itOut->setImag(intbuf[25] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[26] >> trunk_shift);
        itOut->setImag(intbuf[27] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[28] >> trunk_shift);
        itOut->setImag(intbuf[29] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[30] >> trunk_shift);
        itOut->setImag(intbuf[31] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[32] >> trunk_shift);
        itOut->setImag(intbuf[33] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[34] >> trunk_shift);
        itOut->setImag(intbuf[35] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[36] >> trunk_shift);
        itOut->setImag(intbuf[37] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[38] >> trunk_shift);
        itOut->setImag(intbuf[39] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[40] >> trunk_shift);
        itOut->setImag(intbuf[41] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[42] >> trunk_shift);
        itOut->setImag(intbuf[43] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[44] >> trunk_shift);
        itOut->setImag(intbuf[45] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[46] >> trunk_shift);
        itOut->setImag(intbuf[47] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[48] >> trunk_shift);
        itOut->setImag(intbuf[49] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[50] >> trunk_shift);
        itOut->setImag(intbuf[51] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[52] >> trunk_shift);
        itOut->setImag(intbuf[53] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[54] >> trunk_shift);
        itOut->setImag(intbuf[55] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[56] >> trunk_shift);
        itOut->setImag(intbuf[57] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[58] >> trunk_shift);
        itOut->setImag(intbuf[59] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[60] >> trunk_shift);
        itOut->setImag(intbuf[61] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[62] >> trunk_shift);
        itOut->setImag(intbuf[63] >> trunk_shift);
        ++itOut;
    }
}

void Interpolators::interpolate64_cen(unsigned int deviceSampleSize, const IQSampleVector& in, IQSampleVector& out)
{
    int len = in.size();
    out.resize(len*64);
    IQSampleVector::const_iterator itIn = in.begin();
    IQSampleVector::iterator itOut = out.begin();
    unsigned int trunk_shift  = (deviceSampleSize > 16 ? 0 : 16 - deviceSampleSize); // shift to normalize to number of output bits (shift left)
    int32_t intbuf[128];
    memset((void *) &intbuf[64], 0, 64*sizeof(int32_t));

    for (; itIn != in.end(); ++itIn)
    {
        intbuf[0]  = itIn->real();
        intbuf[1]  = itIn->imag();

        m_interpolator2.myInterpolate(&intbuf[0], &intbuf[1], &intbuf[32], &intbuf[33]);

        m_interpolator4.myInterpolate(&intbuf[0],  &intbuf[1],  &intbuf[16], &intbuf[17]);
        m_interpolator4.myInterpolate(&intbuf[32], &intbuf[33], &intbuf[48], &intbuf[49]);

        m_interpolator8.myInterpolate(&intbuf[0],  &intbuf[1],  &intbuf[8],  &intbuf[9]);
        m_interpolator8.myInterpolate(&intbuf[16], &intbuf[17], &intbuf[24], &intbuf[25]);
        m_interpolator8.myInterpolate(&intbuf[32], &intbuf[33], &intbuf[40], &intbuf[41]);
        m_interpolator8.myInterpolate(&intbuf[48], &intbuf[49], &intbuf[56], &intbuf[57]);

        m_interpolator16.myInterpolate(&intbuf[0],  &intbuf[1],  &intbuf[4],  &intbuf[5]);
        m_interpolator16.myInterpolate(&intbuf[8],  &intbuf[9],  &intbuf[12], &intbuf[13]);
        m_interpolator16.myInterpolate(&intbuf[16], &intbuf[17], &intbuf[20], &intbuf[21]);
        m_interpolator16.myInterpolate(&intbuf[24], &intbuf[25], &intbuf[28], &intbuf[29]);
        m_interpolator16.myInterpolate(&intbuf[32], &intbuf[33], &intbuf[36], &intbuf[37]);
        m_interpolator16.myInterpolate(&intbuf[40], &intbuf[41], &intbuf[44], &intbuf[45]);
        m_interpolator16.myInterpolate(&intbuf[48], &intbuf[49], &intbuf[52], &intbuf[53]);
        m_interpolator16.myInterpolate(&intbuf[56], &intbuf[57], &intbuf[60], &intbuf[61]);

        m_interpolator32.myInterpolate(&intbuf[0],  &intbuf[1],  &intbuf[2],  &intbuf[3]);
        m_interpolator32.myInterpolate(&intbuf[4],  &intbuf[5],  &intbuf[6],  &intbuf[7]);
        m_interpolator32.myInterpolate(&intbuf[8],  &intbuf[9],  &intbuf[10], &intbuf[11]);
        m_interpolator32.myInterpolate(&intbuf[12], &intbuf[13], &intbuf[14], &intbuf[15]);
        m_interpolator32.myInterpolate(&intbuf[16], &intbuf[17], &intbuf[18], &intbuf[19]);
        m_interpolator32.myInterpolate(&intbuf[20], &intbuf[21], &intbuf[22], &intbuf[23]);
        m_interpolator32.myInterpolate(&intbuf[24], &intbuf[25], &intbuf[26], &intbuf[27]);
        m_interpolator32.myInterpolate(&intbuf[28], &intbuf[29], &intbuf[30], &intbuf[31]);
        m_interpolator32.myInterpolate(&intbuf[32], &intbuf[33], &intbuf[34], &intbuf[35]);
        m_interpolator32.myInterpolate(&intbuf[36], &intbuf[37], &intbuf[38], &intbuf[39]);
        m_interpolator32.myInterpolate(&intbuf[40], &intbuf[41], &intbuf[42], &intbuf[43]);
        m_interpolator32.myInterpolate(&intbuf[44], &intbuf[45], &intbuf[46], &intbuf[47]);
        m_interpolator32.myInterpolate(&intbuf[48], &intbuf[49], &intbuf[50], &intbuf[51]);
        m_interpolator32.myInterpolate(&intbuf[52], &intbuf[53], &intbuf[54], &intbuf[55]);
        m_interpolator32.myInterpolate(&intbuf[56], &intbuf[57], &intbuf[58], &intbuf[59]);
        m_interpolator32.myInterpolate(&intbuf[60], &intbuf[61], &intbuf[62], &intbuf[63]);

        itOut->setReal(intbuf[0] >> trunk_shift);
        itOut->setImag(intbuf[1] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[2] >> trunk_shift);
        itOut->setImag(intbuf[3] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[4] >> trunk_shift);
        itOut->setImag(intbuf[5] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[6] >> trunk_shift);
        itOut->setImag(intbuf[7] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[8] >> trunk_shift);
        itOut->setImag(intbuf[9] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[10] >> trunk_shift);
        itOut->setImag(intbuf[11] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[12] >> trunk_shift);
        itOut->setImag(intbuf[13] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[14] >> trunk_shift);
        itOut->setImag(intbuf[15] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[16] >> trunk_shift);
        itOut->setImag(intbuf[17] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[18] >> trunk_shift);
        itOut->setImag(intbuf[19] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[20] >> trunk_shift);
        itOut->setImag(intbuf[21] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[22] >> trunk_shift);
        itOut->setImag(intbuf[23] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[24] >> trunk_shift);
        itOut->setImag(intbuf[25] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[26] >> trunk_shift);
        itOut->setImag(intbuf[27] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[28] >> trunk_shift);
        itOut->setImag(intbuf[29] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[30] >> trunk_shift);
        itOut->setImag(intbuf[31] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[32] >> trunk_shift);
        itOut->setImag(intbuf[33] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[34] >> trunk_shift);
        itOut->setImag(intbuf[35] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[36] >> trunk_shift);
        itOut->setImag(intbuf[37] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[38] >> trunk_shift);
        itOut->setImag(intbuf[39] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[40] >> trunk_shift);
        itOut->setImag(intbuf[41] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[42] >> trunk_shift);
        itOut->setImag(intbuf[43] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[44] >> trunk_shift);
        itOut->setImag(intbuf[45] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[46] >> trunk_shift);
        itOut->setImag(intbuf[47] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[48] >> trunk_shift);
        itOut->setImag(intbuf[49] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[50] >> trunk_shift);
        itOut->setImag(intbuf[51] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[52] >> trunk_shift);
        itOut->setImag(intbuf[53] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[54] >> trunk_shift);
        itOut->setImag(intbuf[55] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[56] >> trunk_shift);
        itOut->setImag(intbuf[57] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[58] >> trunk_shift);
        itOut->setImag(intbuf[59] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[60] >> trunk_shift);
        itOut->setImag(intbuf[61] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[62] >> trunk_shift);
        itOut->setImag(intbuf[63] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[64] >> trunk_shift);
        itOut->setImag(intbuf[65] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[66] >> trunk_shift);
        itOut->setImag(intbuf[67] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[68] >> trunk_shift);
        itOut->setImag(intbuf[69] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[70] >> trunk_shift);
        itOut->setImag(intbuf[71] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[72] >> trunk_shift);
        itOut->setImag(intbuf[73] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[74] >> trunk_shift);
        itOut->setImag(intbuf[75] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[76] >> trunk_shift);
        itOut->setImag(intbuf[77] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[78] >> trunk_shift);
        itOut->setImag(intbuf[79] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[80] >> trunk_shift);
        itOut->setImag(intbuf[81] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[82] >> trunk_shift);
        itOut->setImag(intbuf[83] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[84] >> trunk_shift);
        itOut->setImag(intbuf[85] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[86] >> trunk_shift);
        itOut->setImag(intbuf[87] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[88] >> trunk_shift);
        itOut->setImag(intbuf[89] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[90] >> trunk_shift);
        itOut->setImag(intbuf[91] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[92] >> trunk_shift);
        itOut->setImag(intbuf[93] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[94] >> trunk_shift);
        itOut->setImag(intbuf[95] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[96] >> trunk_shift);
        itOut->setImag(intbuf[97] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[98] >> trunk_shift);
        itOut->setImag(intbuf[99] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[100] >> trunk_shift);
        itOut->setImag(intbuf[101] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[102] >> trunk_shift);
        itOut->setImag(intbuf[103] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[104] >> trunk_shift);
        itOut->setImag(intbuf[105] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[106] >> trunk_shift);
        itOut->setImag(intbuf[107] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[108] >> trunk_shift);
        itOut->setImag(intbuf[109] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[110] >> trunk_shift);
        itOut->setImag(intbuf[111] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[112] >> trunk_shift);
        itOut->setImag(intbuf[113] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[114] >> trunk_shift);
        itOut->setImag(intbuf[115] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[116] >> trunk_shift);
        itOut->setImag(intbuf[117] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[118] >> trunk_shift);
        itOut->setImag(intbuf[119] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[120] >> trunk_shift);
        itOut->setImag(intbuf[121] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[122] >> trunk_shift);
        itOut->setImag(intbuf[123] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[124] >> trunk_shift);
        itOut->setImag(intbuf[125] >> trunk_shift);
        ++itOut;
        itOut->setReal(intbuf[126] >> trunk_shift);
        itOut->setImag(intbuf[127] >> trunk_shift);
        ++itOut;
    }
}
