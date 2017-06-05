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
#include <ctime>
#include <unistd.h>

#include "FileSink.h"
#include "util.h"
#include "parsekv.h"
#include "UDPSource.h"

FileSink *FileSink::m_this = 0;

FileSink::FileSink() :
    m_sampleRate(192000),
    m_frequency(435000000),
    m_filename("test.sdriq"),
    m_delayUS(1000000),
    m_running(false),
    m_thread(0),
    m_iqSamplesIndex(0)
{
    m_devname = "FileSink";
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
    devices.push_back("file");
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
    fprintf(stderr, "File name:         %s\n", m_filename.c_str());
}

bool FileSink::configure(uint32_t changeFlags,
        uint32_t sample_rate,
		uint64_t frequency
)
{
    bool closeAndOpenFlag = false;

    if (changeFlags & 0x1)
    {
    	m_frequency = frequency;
    	closeAndOpenFlag = true;
    }

    if (changeFlags & 0x2)
    {
        m_sampleRate = sample_rate;
        double delayF = (127*127*750000.0) / m_sampleRate; // 0.75 factor
        m_delayUS = (uint32_t) delayF;
        closeAndOpenFlag = true;
    }

    if (changeFlags & 0x4) // file name
    {
        closeAndOpenFlag = true;
    }

    if (changeFlags & 0x8) // interpolation factor
    {
        closeAndOpenFlag = true;
    }

    if (closeAndOpenFlag) closeAndOpen();

    return true;
}

bool FileSink::configure(parsekv::pairs_type& m)
{
    uint32_t sampleRate = 48000;
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

	if (m.find("file") != m.end())
	{
		std::cerr << "FileSink::configure: file: " << m["file"] << std::endl;
        m_filename = m["file"];
        changeFlags |= 0x4;
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
            changeFlags |= 0x8;
        }
    }

    m_confFreq = frequency;
	double tuner_freq = frequency;

    return configure(changeFlags, sampleRate, tuner_freq);
}

void FileSink::closeAndOpen()
{
    if (m_ofstream.is_open()) {
        m_ofstream.close();
    }

    m_ofstream.open(m_filename.c_str(),  std::ios::binary);

    m_ofstream.write((const char *) &m_sampleRate, sizeof(int));
    m_ofstream.write((const char *) &m_frequency, sizeof(uint64_t));
    std::time_t startingTimeStamp = time(0);
    m_ofstream.write((const char *) &startingTimeStamp, sizeof(std::time_t));

    fprintf(stderr, "FileSink::closeAndOpen: %s %u %lu\n", m_filename.c_str(), m_sampleRate, m_frequency);
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
    unsigned int count = 0;
    char msgBufSend[128];

    if (!m_this->m_ofstream.is_open()) m_this->closeAndOpen();

    while (!stop_flag->load())
    {
        int len = nn_recv(m_this->m_nnReceiver, &msgBuf, NN_MSG, NN_DONTWAIT);

        if ((len > 0) && msgBuf)
        {
            std::string msg((char *) msgBuf, len);
            std::cerr << "FileSink::run: received message: " << msg << std::endl;
            bool success = m_this->DeviceSink::configure(msg);
            nn_freemsg(msgBuf);
            msgBuf = 0;
            if (!success) {
                std::cerr << "FileSink::run: config error: " << m_this->DeviceSink::error() << std::endl;
            }
        }

        if (m_this->m_buf->queued_samples() > 0)
        {
            fprintf(stderr, "FileSink::run: %lu samples left in queue\n", m_this->m_buf->queued_samples());
            m_this->m_iqSamples = m_this->m_buf->pull();
            m_this->m_ofstream.write(reinterpret_cast<char*>(&(m_this->m_iqSamples[0])), m_this->m_iqSamples.size()*2*sizeof(int16_t));
        }

        if (count < 1000000 / m_this->m_delayUS)
        {
            count++;
        }
        else
        {
            uint32_t queuedVectors = m_this->m_buf->queued_vectors();
            sprintf(msgBufSend, "%u", queuedVectors);

            if (m_this->m_udpSource)
            {
                m_this->m_udpSource->getStatusMessage(msgBufSend);
            }

            int bufSize = strlen(msgBufSend);
            int rc = nn_send(m_this->m_nnReceiver, (void *) msgBufSend, bufSize, 0);

            if (rc != bufSize)
            {
                std::cerr << "HackRFSink::run: Cannot send message: " << msgBufSend << std::endl;
            }

            count = 0;
        }

        usleep(m_this->m_delayUS);
    }

    m_this->m_ofstream.close();
}

bool FileSink::stop()
{
    std::cerr << "FileSink::stop" << std::endl;

    m_thread->join();
    delete m_thread;
    return true;
}
