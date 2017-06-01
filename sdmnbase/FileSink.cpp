///////////////////////////////////////////////////////////////////////////////////
// SDRdaemon - receive I/Q samples over the network via UDP and write to a       //
//             SDR device .                                                      //
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

#include <cstring>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <thread>
#include <cstdlib>

#include "FileSink.h"
#include "util.h"
#include "parsekv.h"

FileSink *FileSink::m_this = 0;

FileSink::FileSink(int dev_index) :
    m_sampleRate(192000),
    m_frequency(435000000),
    m_running(false),
    m_thread(0),
    m_iqSamplesIndex(0)
{
    m_devname = "FileSink";

//    hackrf_error rc = (hackrf_error) hackrf_init();
//
//    if (rc != HACKRF_SUCCESS)
//    {
//        std::ostringstream err_ostr;
//        err_ostr << "HackRFSink::HackRFSink: failed to open HackRF library (" << rc << ": " << hackrf_error_name(rc) << ")";
//        m_error = err_ostr.str();
//        m_dev = 0;
//    }
//    else
//    {
//        hackrf_device_list_t *hackrf_devices = hackrf_device_list();
//
//        rc = (hackrf_error) hackrf_device_list_open(hackrf_devices, dev_index, &m_dev);
//
//        if (rc != HACKRF_SUCCESS)
//        {
//            std::ostringstream err_ostr;
//            err_ostr << "HackRFSink::HackRFSink: failed to open HackRF device " << dev_index << " (" << rc << ": " << hackrf_error_name(rc) << ")";
//            m_error = err_ostr.str();
//            m_dev = 0;
//        }
//    }

    m_this = this;
}

FileSink::~FileSink()
{
//    if (m_dev) {
//        hackrf_close(m_dev);
//    }

    m_this = 0;
}

void FileSink::get_device_names(std::vector<std::string>& devices)
{
}

std::uint32_t FileSink::get_sample_rate()
{
    return m_sampleRate;
}

std::uint32_t FileSink::get_frequency()
{
    return m_frequency;
}

void FileSink::print_specific_parms()
{
}

bool FileSink::configure(uint32_t changeFlags,
        uint32_t sample_rate,
		uint64_t frequency
)
{
    if (changeFlags & 0x1)
    {
    	m_frequency = frequency;

//        rc = (hackrf_error) hackrf_set_freq(m_dev, m_frequency);
//
//        if (rc != HACKRF_SUCCESS)
//        {
//            std::ostringstream err_ostr;
//            err_ostr << "HackRFSink::configure(flags): could not set center frequency to " << m_frequency << " Hz";
//            m_error = err_ostr.str();
//            return false;
//        }
//        else
//        {
//            std::cerr << "HackRFSink::configure(flags): set center frequency to " << m_frequency << " Hz" << std::endl;
//        }
    }

    if (changeFlags & 0x2)
    {
        m_sampleRate = sample_rate;

//        rc = (hackrf_error) hackrf_set_sample_rate_manual(m_dev, sample_rate, 1);
//
//        if (rc != HACKRF_SUCCESS)
//        {
//            std::ostringstream err_ostr;
//            err_ostr << "HackRFSink::configure(flags): could not set sample rate to " << sample_rate << " Hz";
//            m_error = err_ostr.str();
//            return false;
//        }
//        else
//        {
//            std::cerr << "HackRFSink::configure(flags): set sample rate to " << sample_rate << " Hz" << std::endl;
//            m_sampleRate = sample_rate;
//        }
    }

    return true;
}

bool FileSink::configure(parsekv::pairs_type& m)
{
    uint32_t sampleRate = 5000000;
    uint64_t frequency = m_confFreq;
    std::uint32_t changeFlags = 0;

	if (m.find("srate") != m.end())
	{
		std::cerr << "FileSink::configure: srate: " << m["srate"] << std::endl;
		sampleRate = atoi(m["srate"].c_str());

		if ((sampleRate < 48000) || (sampleRate > 5000000))
		{
		    std::cerr << "FileSink::configure: Invalid sample rate " << sampleRate << " skipping" << std::endl;
		}
		else
		{
	        changeFlags |= 0x2;
		}
	}

	if (m.find("freq") != m.end())
	{
		std::cerr << "FileSink::configure: freq: " << m["freq"] << std::endl;
		frequency = strtoll(m["freq"].c_str(), 0, 10);

		if ((frequency < 1000000) || (frequency > 6000000000))
		{
            std::cerr << "FileSink::configure: Invalid frequency " << frequency << " skipping" << std::endl;
		}
		else
		{
	        changeFlags |= 0x1;
		}
	}

	if (m.find("interp") != m.end())
	{
		std::cerr << "FileSink::configure: interp: " << m["interp"] << std::endl;
		int log2Interp = atoi(m["interp"].c_str());

		if ((log2Interp < 0) || (log2Interp > 6))
		{
		    std::cerr << "FileSink::configure: Invalid log2 interpolation factor" << std::endl;
		}
		else
		{
			m_interp = log2Interp;
		}
	}

    m_confFreq = frequency;
	double tuner_freq;

	// Centered
	tuner_freq = frequency;

    return configure(changeFlags, sampleRate, tuner_freq);
}

bool FileSink::start(DataBuffer<IQSample> *buf, std::atomic_bool *stop_flag)
{
    m_buf = buf;
    m_stop_flag = stop_flag;

    if (m_thread == 0)
    {
        std::cerr << "FileSink::start: starting" << std::endl;
        m_running = true;
        m_thread = new std::thread(run, stop_flag);
        sleep(1);
        return *this;
    }
    else
    {
        std::cerr << "FileSink::start: error" << std::endl;
        m_error = "Sink thread already started";
        return false;
    }
}

void FileSink::run(std::atomic_bool *stop_flag)
{
    std::cerr << "FileSink::run" << std::endl;
    void *msgBuf = 0;

//    while (!stop_flag->load())
//    {
//        sleep(1);
//
//        int len = nn_recv(m_this->m_nnReceiver, &msgBuf, NN_MSG, NN_DONTWAIT);
//
//        if ((len > 0) && msgBuf)
//        {
//            std::string msg((char *) msgBuf, len);
//            std::cerr << "HackRFSink::run: received message: " << msg << std::endl;
//            bool success = m_this->Sink::configure(msg);
//            nn_freemsg(msgBuf);
//            msgBuf = 0;
//            if (!success) {
//                std::cerr << "HackRFSink::run: config error: " << m_this->Sink::error() << std::endl;
//            }
//        }
//    }
//
//    std::cerr << "HackRFSink::run: finished" << std::endl;

//    hackrf_error rc = (hackrf_error) hackrf_start_tx(dev, tx_callback, 0);
//
//    if (rc == HACKRF_SUCCESS)
//    {
//        while (!stop_flag->load() && (hackrf_is_streaming(dev) == HACKRF_TRUE))
//        {
//            sleep(1);
//
//            int len = nn_recv(m_this->m_nnReceiver, &msgBuf, NN_MSG, NN_DONTWAIT);
//
//            if ((len > 0) && msgBuf)
//            {
//                std::string msg((char *) msgBuf, len);
//                std::cerr << "HackRFSink::run: received message: " << msg << std::endl;
//                bool success = m_this->DeviceSink::configure(msg);
//                nn_freemsg(msgBuf);
//                msgBuf = 0;
//                if (!success) {
//                    std::cerr << "HackRFSink::run: config error: " << m_this->DeviceSink::error() << std::endl;
//                }
//            }
//
//            //std::cerr << "HackRFSink::run..." << std::endl;
//        }
//
//        std::cerr << "HackRFSink::run: finished" << std::endl;
//
//        rc = (hackrf_error) hackrf_stop_tx(dev);
//
//        if (rc != HACKRF_SUCCESS)
//        {
//            std::cerr << "HackRFSink::run: Cannot stop HackRF Tx: " << rc << ": " << hackrf_error_name(rc) << std::endl;
//        }
//    }
//    else
//    {
//        std::cerr << "HackRFSink::run: Cannot start HackRF Tx: " << rc << ": " << hackrf_error_name(rc) << std::endl;
//    }
}

bool FileSink::stop()
{
    std::cerr << "FileSink::stop" << std::endl;

    m_thread->join();
    delete m_thread;
    return true;
}

int FileSink::tx_callback()
{
//    int bytes_to_read = transfer->valid_length; // bytes to read from FIFO as expected by the Tx
//
//    if (m_this)
//    {
//        m_this->callback((char *) transfer->buffer, bytes_to_read);
//    }
//
//    return 0;
}

void FileSink::callback(char* buf, int len)
{
    int i = 0;

    for (; i < len/2; i++)
    {
        if (m_iqSamplesIndex < m_iqSamples.size())
        {
            buf[2*i]     = 8;
            buf[2*i+1]   = 0;
//            buf[2*i]     = m_iqSamples[m_iqSamplesIndex].real() >> 4;
//            buf[2*i+1]   = m_iqSamples[m_iqSamplesIndex].imag() >> 4;
            m_iqSamplesIndex++;
        }
        else
        {
            if (m_buf->test_buffer_fill((len/2) - m_iqSamplesIndex))
            {
                m_iqSamples = m_buf->pull();
                fprintf(stderr, "HackRFSink::callback: len: %d, pull size: %lu, queue size: %lu\n", len, m_iqSamples.size(), m_buf->queued_samples());
                m_iqSamplesIndex = 0;
            }
            else
            {
                m_iqSamples.clear();
                m_iqSamplesIndex = 0;
                break;
            }
        }
    }

    for (; i < len/2; i++)
    {
        buf[2*i]     = 8;
        buf[2*i+1]   = 0;
    }
}
