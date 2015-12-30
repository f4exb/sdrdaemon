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

#include "Downsampler.h"

Downsampler::Downsampler(unsigned int decim,
		fcPos_t fcPos) :
	m_decim(decim),
	m_fcPos(fcPos)
{
}

Downsampler::~Downsampler()
{
}

bool Downsampler::configure(parsekv::pairs_type& m)
{
	if (m.find("decim") != m.end())
	{
		std::cerr << "Downsampler::configure: decim: " << m["decim"] << std::endl;
		int log2Decim = atoi(m["decim"].c_str());

		if ((log2Decim < 0) || (log2Decim > 6))
		{
			m_error = "Invalid log2 decimation factor";
			return false;
		}
		else
		{
			m_decim = log2Decim;
		}
	}

	if (m.find("fcpos") != m.end())
	{
		std::cerr << "Downsampler::configure: fcpos: " << m["fcpos"] << std::endl;
		int fcPosIndex = atoi(m["fcpos"].c_str());

		if ((fcPosIndex < (int) FC_POS_INFRA) || (fcPosIndex > (int) FC_POS_CENTER))
		{
			m_error = "Invalid Fc position index";
			return false;
		}
		else
		{
			m_fcPos = (fcPos_t) fcPosIndex;
		}
	}

	return true;
}

void Downsampler::process(unsigned int& sampleSize, const IQSampleVector& samples_in, IQSampleVector& samples_out)
{
	if (m_decim == 0)
	{
		samples_out = samples_in;
	}
	else
	{
		if (m_fcPos == 0) // infra
		{
			switch (m_decim)
			{
			case 1:
				Decimators::decimate2_inf(sampleSize, samples_in, samples_out);
				break;
			case 2:
				Decimators::decimate4_inf(sampleSize, samples_in, samples_out);
				break;
			case 3:
				m_decimators.decimate8_inf(sampleSize, samples_in, samples_out);
				break;
			case 4:
				m_decimators.decimate16_inf(sampleSize, samples_in, samples_out);
				break;
			case 5:
				m_decimators.decimate32_inf(sampleSize, samples_in, samples_out);
				break;
			case 6:
				m_decimators.decimate64_inf(sampleSize, samples_in, samples_out);
				break;
			default:
				break;
			}
		}
		else if (m_fcPos == 1) // supra
		{
			switch (m_decim)
			{
			case 1:
				Decimators::decimate2_sup(sampleSize, samples_in, samples_out);
				break;
			case 2:
				Decimators::decimate4_sup(sampleSize, samples_in, samples_out);
				break;
			case 3:
				m_decimators.decimate8_sup(sampleSize, samples_in, samples_out);
				break;
			case 4:
				m_decimators.decimate16_sup(sampleSize, samples_in, samples_out);
				break;
			case 5:
				m_decimators.decimate32_sup(sampleSize, samples_in, samples_out);
				break;
			case 6:
				m_decimators.decimate64_sup(sampleSize, samples_in, samples_out);
				break;
			default:
				break;
			}
		}
		else // centered
		{
			switch (m_decim)
			{
			case 1:
				m_decimators.decimate2_cen(sampleSize, samples_in, samples_out);
				break;
			case 2:
				m_decimators.decimate4_cen(sampleSize, samples_in, samples_out);
				break;
			case 3:
				m_decimators.decimate8_cen(sampleSize, samples_in, samples_out);
				break;
			case 4:
				m_decimators.decimate16_cen(sampleSize, samples_in, samples_out);
				break;
			case 5:
				m_decimators.decimate32_cen(sampleSize, samples_in, samples_out);
				break;
			case 6:
				m_decimators.decimate64_cen(sampleSize, samples_in, samples_out);
				break;
			default:
				break;
			}
		}
	}
}
