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

#ifndef INCLUDE_UDPSINK_H_
#define INCLUDE_UDPSINK_H_

#include "SDRDaemon.h"

/** Downsampler and UDP sink for samples read from the device */
class UDPSink
{
public:
	/**
	 * Construct UDP sink
	 *
	 * port             :: UDP port where the samples are sent
	 */
	UDPSink(unsigned int port);

	/** Destroy UDP sink */
	~UDPSink();

    /**
     * Write IQ samples to UDP port.
     */
	void write(const IQSampleVector& samples_in);

    /** Return the last error, or return an empty string if there is no error. */
    std::string error()
    {
        std::string ret(m_error);
        m_error.clear();
        return ret;
    }

    /** Return true if the stream is OK, return false if there is an error. */
    operator bool() const
    {
        return (!m_zombie) && m_error.empty();
    }

private:
	unsigned int m_port;
    std::string  m_error;
    bool         m_zombie;
};



#endif /* INCLUDE_UDPSINK_H_ */
