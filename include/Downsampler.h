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

#ifndef INCLUDE_DOWNSAMPLER_H_
#define INCLUDE_DOWNSAMPLER_H_

#include "SDRDaemon.h"
#include "parsekv.h"

class Downsampler
{
public:
	/** Center frequency relative position when downsampling */
	typedef enum {
		FC_POS_INFRA = 0,
		FC_POS_SUPRA,
		FC_POS_CENTER
	} fcPos_t;

	/**
	 * Construct Downsampler
	 *
     * decim            :: log2 of decimation factor
     * fcpos            :: Position of center frequency
	 */
	Downsampler(unsigned int decim = 0,
			fcPos_t fcPos = FC_POS_CENTER);

	/** Destroy Downsampler */
	~Downsampler();

	/** Configure downsampler dynamically */
	bool configure(parsekv::pairs_type& m);

	/** Return log2 of decimation */
	bool getLog2Decimation() const { return m_decim; }

    /**
     * Process samples.
     */
    void process(unsigned int& sampleSize, const IQSampleVector& samples_in, IQSampleVector& samples_out);

    /** State operator */
    operator bool() const
	{
    	return m_error.empty();
	}

    /** Return the last error, or return an empty string if there is no error. */
    std::string error()
    {
        std::string ret(m_error);
        m_error.clear();
        return ret;
    }

private:
    unsigned int m_decim;
    fcPos_t      m_fcPos;
    std::string  m_error;
};

#endif /* INCLUDE_DOWNSAMPLER_H_ */
