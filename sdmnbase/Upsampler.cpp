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

#include "Upsampler.h"

Upsampler::Upsampler(unsigned int interp) :
	m_interp(interp)
{
}

Upsampler::~Upsampler()
{
}

bool Upsampler::configure(parsekv::pairs_type& m)
{
	if (m.find("interp") != m.end())
	{
		std::cerr << "Upsampler::configure: interp: " << m["interp"] << std::endl;
		int log2Interp = atoi(m["interp"].c_str());

		if ((log2Interp < 0) || (log2Interp > 6))
		{
			m_error = "Invalid log2 interpolation factor";
			return false;
		}
		else
		{
		    m_interp = log2Interp;
		}
	}

	return true;
}

void Upsampler::process(unsigned int& sampleSize, const IQSampleVector& samples_in, IQSampleVector& samples_out)
{
	if (m_interp == 0)
	{
		samples_out = samples_in;
	}
	else
	{
        switch (m_interp)
        {
        case 1:
            m_interpolators.interpolate2_cen(sampleSize, samples_in, samples_out);
            break;
        case 2:
            m_interpolators.interpolate4_cen(sampleSize, samples_in, samples_out);
            break;
        case 3:
            m_interpolators.interpolate8_cen(sampleSize, samples_in, samples_out);
            break;
        case 4:
            m_interpolators.interpolate16_cen(sampleSize, samples_in, samples_out);
            break;
        case 5:
            m_interpolators.interpolate32_cen(sampleSize, samples_in, samples_out);
            break;
        case 6:
            m_interpolators.interpolate64_cen(sampleSize, samples_in, samples_out);
            break;
        default:
            break;
        }
	}
}
