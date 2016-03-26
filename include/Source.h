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

#ifndef INCLUDE_SOURCE_H_
#define INCLUDE_SOURCE_H_

#include <string>
#include <atomic>
#include <memory>
#include <iostream>
#include <sstream>
#include <cassert>

#include "nanomsg/nn.h"
#include "nanomsg/pair.h"

#include "parsekv.h"
#include "DataBuffer.h"
#include "SDRDaemon.h"

class Downsampler;

class Source
{
public:
    Source() : m_confFreq(0),
	    m_decim(0),
		m_fcPos(2),
		m_buf(0),
		m_downsampler(0)
    {
        m_nnReceiver = nn_socket(AF_SP, NN_PAIR);
        assert(m_nnReceiver != -1);
    }

    virtual ~Source() {}

    /** Associate with a Downsampler. The Downsampler will be configured
     *  dynamically from the source
     */
    void associateDownsampler(Downsampler *downsampler)
    {
        m_downsampler = downsampler;
    }

    /** set the TCP port used by 0MQ to receive configuration messages */
    void setConfigurationPort(std::uint32_t ctlPort)
    {
    	if ((ctlPort < 1024) || (ctlPort > 65535))
    	{
    		ctlPort = 9091;
    	}

    	std::ostringstream os;
    	os << "tcp://*:" << ctlPort;

        int rc = nn_bind(m_nnReceiver, os.str().c_str());
        assert(rc >= 0);
    }

    /**
     * Configure device and prepare for streaming.
     */
    bool configure(std::string& configureStr);

    /** Return sample size in bits */
    virtual std::uint32_t get_sample_bits() = 0;

    /** Return current sample frequency in Hz. */
    virtual std::uint32_t get_sample_rate() = 0;

    /** Return device current center frequency in Hz. */
    virtual std::uint32_t get_frequency() = 0;

    /** Return current received center frequency in Hz.
     *  Actual device frequency depends on center frequency relative position
     *  configured in the downsampler */
    std::uint64_t get_received_frequency() const
    {
        return m_confFreq;
    }

    /** Print current parameters specific to device type */
    virtual void print_specific_parms() = 0;

    /** start device before sampling loop.
     * Give it a reference to the buffer of samples */
    virtual bool start(DataBuffer<IQSample> *buf, std::atomic_bool *stop_flag) = 0;

    /** stop device after sampling loop */
    virtual bool stop() = 0;

    /** Return true if the device is OK, return false if there is an error. */
    virtual operator bool() const = 0;

    /** Return name of opened RTL-SDR device. */
    std::string get_device_name() const
    {
        return m_devname;
    }

    /** Return the last error, or return an empty string if there is no error. */
    std::string error()
    {
        std::string ret(m_error);
        m_error.clear();
        return ret;
    }

protected:
    std::string           m_devname;
    std::string           m_error;
    uint64_t              m_confFreq;
    unsigned int          m_decim;
    int                   m_fcPos;
    DataBuffer<IQSample> *m_buf;
    std::atomic_bool     *m_stop_flag;
    Downsampler          *m_downsampler;
    int                   m_nnReceiver; //!< nanomsg socket handle


    /** Configure device and prepare for streaming from parameters map */
    virtual bool configure(parsekv::pairs_type& m) = 0;
};

#endif /* INCLUDE_SOURCE_H_ */
