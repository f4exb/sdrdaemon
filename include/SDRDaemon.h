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

#ifndef SOFTFM_H
#define SOFTFM_H

#include <cstdint>
#include <vector>

typedef std::int16_t FixReal;

#pragma pack(push, 1)
struct IQSample
{
	IQSample() : m_real(0), m_imag(0) {}
	IQSample(FixReal real) : m_real(real), m_imag(0) {}
	IQSample(FixReal real, FixReal imag) : m_real(real), m_imag(imag) {}
	IQSample(const IQSample& other) : m_real(other.m_real), m_imag(other.m_imag) {}
	inline IQSample& operator=(const IQSample& other) { m_real = other.m_real; m_imag = other.m_imag; return *this; }

	inline IQSample& operator+=(const IQSample& other) { m_real += other.m_real; m_imag += other.m_imag; return *this; }
	inline IQSample& operator-=(const IQSample& other) { m_real -= other.m_real; m_imag -= other.m_imag; return *this; }

	inline void setReal(FixReal v) { m_real = v; }
	inline void setImag(FixReal v) { m_imag = v; }

	inline FixReal real() const { return m_real; }
	inline FixReal imag() const { return m_imag; }

	FixReal m_real;
	FixReal m_imag;
};
#pragma pack(pop)

typedef std::vector<IQSample> IQSampleVector;
typedef std::vector<FixReal> SampleVector;

#endif
