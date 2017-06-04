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

#ifndef INCLUDE_UPSAMPLER_H_
#define INCLUDE_UPSAMPLER_H_

#include "Interpolators.h"
#include "SDRDaemon.h"
#include "parsekv.h"

class Upsampler
{
public:
    /**
     * Construct Upsampler
     *
     * interp           :: log2 of interpolation factor
     * fcpos            :: Position of center frequency
     */
    Upsampler(unsigned int interp = 0);

    /** Destroy Upsampler */
    ~Upsampler();

    /** Configure upsampler dynamically */
    bool configure(parsekv::pairs_type& m);

    /** Return log2 of interpolation */
    unsigned int getLog2Interpolation() const { return m_interp; }

    /**
     * Process samples.
     */
    void process(const IQSampleVector& samples_in, IQSampleVector& samples_out);

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
    unsigned int  m_interp;
    Interpolators m_interpolators;
    std::string   m_error;
};

#endif /* INCLUDE_UPSAMPLER_H_ */
