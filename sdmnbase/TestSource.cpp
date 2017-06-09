///////////////////////////////////////////////////////////////////////////////////
// SDRdaemon - send I/Q samples read from a SDR device over the network via UDP. //
//                                                                               //
// Copyright (C) 2016 Edouard Griffiths, F4EXB                                   //
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

#include <climits>
#include <cstring>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <thread>
#include <unistd.h>

#include "TestSource.h"
#include "util.h"
#include "parsekv.h"

TestSource *TestSource::m_this = 0;
const uint32_t TestSource::m_sampleBits = 15;
const uint32_t TestSource::m_sampleWidth = 1<<TestSource::m_sampleBits;
const uint32_t TestSource::m_sampleHalfWidth = 1<<(TestSource::m_sampleBits-1);

// Open test device.
TestSource::TestSource(int dev_index) :
    m_dev(dev_index),
    m_block_length(default_block_length),
    m_thread(0),
    m_carrierOffset(10000),
	m_phase(0.0),
	m_freq(435000000),
	m_srate(64000),
	m_amplitude(0.1)
{
    m_this = this;
    m_confFreq = 435000000; // default frequency center position in Source.h is centered
    m_deltaPhase = getDeltaPhase(m_carrierOffset, m_srate);
}


// Close test device.
TestSource::~TestSource()
{
    m_this = 0;
}

bool TestSource::configure(parsekv::pairs_type& m)
{
    uint32_t sample_rate = m_srate;
    uint32_t frequency = m_confFreq;
    int32_t carrierOffset = m_carrierOffset;
    float deltaPhase = m_deltaPhase;
    float amplitude = 1.0;
    int block_length =  default_block_length;
    int fcpos = 2; // default is center
    std::uint32_t changeFlags = 0;
    bool dfp = false;

	if (m.find("srate") != m.end())
	{
		std::cerr << "TestSource::configure(m): srate: " << m["srate"] << std::endl;
		sample_rate = atoi(m["srate"].c_str());

		if ((sample_rate < 8000) || (sample_rate > 10000000))
		{
			m_error = "Invalid sample rate";
			return false;
		}

        deltaPhase = getDeltaPhase(carrierOffset, sample_rate);

		changeFlags |= 0x5; // change delta phase and sample rate

        if (m_fcPos != 2)
        {
            changeFlags |= 0x2; // need to adjust actual center frequency if not centered
        }
	}

	if (m.find("freq") != m.end())
	{
		std::cerr << "TestSource::configure(m): freq: " << m["freq"] << std::endl;
		frequency = atoi(m["freq"].c_str());

		if (frequency < 10000)
		{
			m_error = "Invalid frequency";
			return false;
		}

		changeFlags |= 0x2;
	}

	if (m.find("dfp") != m.end())
	{
		std::cerr << "TestSource::configure(m): dfp: " << m["dfp"] << std::endl;
		int32_t carrierOffset = atoi(m["dfp"].c_str());

		if ((carrierOffset > (int32_t) sample_rate/2) || (carrierOffset < 0))
		{
			m_error = "Invalid positive carrier offset";
			return false;
		}
		else
		{
			deltaPhase = getDeltaPhase(carrierOffset, sample_rate);
		}

		dfp = true;
		changeFlags |= 0x4;
	}

	if ((m.find("dfn") != m.end()) && !dfp)
	{
		std::cerr << "TestSource::configure(m): dfn: " << m["dfn"] << std::endl;
		int32_t carrierOffset = atoi(m["dfn"].c_str());

		if ((carrierOffset > (int32_t) sample_rate/2) || (carrierOffset < 0))
		{
			m_error = "Invalid negative carrier offset";
			return false;
		}
		else
		{
			deltaPhase = getDeltaPhase(-carrierOffset, sample_rate);
		}

		changeFlags |= 0x4;
	}

	if (m.find("power") != m.end())
	{
		std::string gain_str = m["power"];
		std::cerr << "TestSource::configure(m): power: " << gain_str << std::endl;
		int dbn = atoi(m["power"].c_str());

		if (dbn < 0)
		{
			m_error = "Invalid peak power";
			return false;
		}

		amplitude = db2A(-dbn);

		changeFlags |= 0x8;
	} // gain

	if (m.find("blklen") != m.end())
	{
		std::cerr << "TestSource::configure(m): blklen: " << m["blklen"] << std::endl;
		block_length = atoi(m["blklen"].c_str());

		changeFlags |= 0x20;
	}

	if (m.find("fcpos") != m.end())
	{
		std::cerr << "TestSource::configure(m): fcpos: " << m["fcpos"] << std::endl;
		fcpos = atoi(m["fcpos"].c_str());

		if ((fcpos < 0) || (fcpos > 2))
		{
			m_error = "Invalid center frequency position";
			return false;
		}
		else
		{
			m_fcPos = fcpos;
		}

        changeFlags |= 0x2; // need to adjust actual center frequency if not centered
	}

	if (m.find("decim") != m.end())
	{
		std::cerr << "TestSource::configure(m): decim: " << m["decim"] << std::endl;
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

	// Intentionally tune at a higher frequency to avoid DC offset.
	m_confFreq = frequency;
	double tuner_freq;

	if (m_fcPos == 0) { // Infradyne
		tuner_freq = frequency + 0.25 * sample_rate;
	} else if (m_fcPos == 1) { // Supradyne
		tuner_freq = frequency - 0.25 * sample_rate;
	} else { // Centered
		tuner_freq = frequency;
	}

	return configure(changeFlags, sample_rate, tuner_freq, carrierOffset, deltaPhase, amplitude, block_length);
}

// Configure test generator.
bool TestSource::configure(std::uint32_t changeFlags,
		std::uint32_t sample_rate,
		std::uint32_t frequency,
        std::int32_t carrierOffset,
		float deltaPhase,
        float amplitude,
        int block_length)
{
    if (changeFlags & 0x1)
    {
    	m_srate = sample_rate;
    }

    if (changeFlags & 0x2)
    {
    	m_freq = frequency;
    }

    if (changeFlags & 0x4)
    {
    	m_deltaPhase = deltaPhase;
        m_carrierOffset = carrierOffset;
    }

    if (changeFlags & 0x8)
    {
    	m_amplitude = amplitude;
    }

    if (changeFlags & 0x20)
    {
	   // set block length
		m_block_length = (block_length < 4096) ? 4096 :
						 (block_length > 1024 * 1024) ? 1024 * 1024 :
						 block_length;
    }

    return true;
}


// Return current sample frequency in Hz.
uint32_t TestSource::get_sample_rate()
{
    return m_srate;
}

// Return device current center frequency in Hz.
uint32_t TestSource::get_frequency()
{
    return m_freq;
}

void TestSource::print_specific_parms()
{
	std::cerr << "Delta phase:       " << m_deltaPhase << " radians" << std::endl;
	std::cerr << "Amplitude:         " << m_amplitude << std::endl;
}

bool TestSource::start(DataBuffer<IQSample>* buf, std::atomic_bool *stop_flag)
{
	std::cerr << "TestSource::start" << std::endl;

    m_buf = buf;
    m_stop_flag = stop_flag;

    if (m_thread == 0)
    {
        m_thread = new std::thread(run);
        return true;
    }
    else
    {
        m_error = "Source thread already started";
        return false;
    }
}

bool TestSource::stop()
{
	std::cerr << "TestSource::stop" << std::endl;

	if (m_thread)
    {
        m_thread->join();
        delete m_thread;
        m_thread = 0;
    }

    return true;
}

void TestSource::run()
{
	std::cerr << "TestSource::run" << std::endl;

    IQSampleVector iqsamples;
    void *msgBuf = 0;

    while (!m_this->m_stop_flag->load() && get_samples(&iqsamples))
    {
        m_this->m_buf->push(move(iqsamples));

        int len = nn_recv(m_this->m_nnReceiver, &msgBuf, NN_MSG, NN_DONTWAIT);

        if ((len > 0) && msgBuf)
        {
            std::string msg((char *) msgBuf, len);
            std::cerr << "TestSource::run: received: " << msg << std::endl;
            m_this->DeviceSource::configure(msg);
            nn_freemsg(msgBuf);
            msgBuf = 0;
        }
    }
}

// Fetch a bunch of samples from the device.
bool TestSource::get_samples(IQSampleVector *samples)
{
    int r, n_read;

    if (!samples) {
        return false;
    }

    std::vector<int16_t> buf(2 * m_this->m_block_length); // one sample for I, one sample for Q

    r = read_samples(buf.data(), 4 * m_this->m_block_length, n_read, m_this->m_phase, m_this->m_srate, m_this->m_deltaPhase, m_this->m_amplitude);

    if (r < 0)
    {
        m_this->m_error = "TestSource::get_samples: read_samples failed";
        return false;
    }

    if (n_read != 4 * m_this-> m_block_length)
    {
        m_this->m_error = "TestSource::get_samples: short read, samples lost";
        return false;
    }

    samples->resize(m_this->m_block_length);

    for (int i = 0; i < m_this->m_block_length; i++)
    {
        (*samples)[i] = IQSample(buf[2*i], buf[2*i+1]);
    }

    return true;
}


// Return a list of supported devices.
void TestSource::get_device_names(std::vector<std::string>& devices)
{
    char manufacturer[256], product[256], serial[256];
    int device_count = 1;

    if (device_count > 0)
    {
        devices.resize(device_count);
    }

    devices.clear();

    for (int i = 0; i < device_count; i++)
    {
    	strcpy(manufacturer, "Test");
    	strcpy(product, "Test dummy device");
    	strcpy(serial, "0000001");

        std::ostringstream name_ostr;
        name_ostr << manufacturer << " " << product << " " << serial << " " << i;
        devices.push_back(name_ostr.str());
    }
}

int TestSource::read_samples(int16_t* data, int iqBlockSize, int& getSize, float& phasor, int sampleRate, float deltaPhase, float amplitude)
{
	int nbSamples = iqBlockSize / 4; // 16 bit samples
	float dt = (float) nbSamples / (float) sampleRate;
	int dtMicroseconds = (int) (dt * 1e6);

	int i = 0;

	for (; i < nbSamples * 2; i += 2)
	{
        data[i]   = amplitude * cos(phasor) * m_sampleHalfWidth;
        data[i+1] = amplitude * sin(phasor) * m_sampleHalfWidth;

		//phasor += 2.0 * M_PI * (1 / 20.0);
		phasor += deltaPhase;

		if (phasor > 2.0 * M_PI) {
			phasor -= 2.0 * M_PI;
		} else if (phasor < 2.0 * M_PI) {
			phasor += 2.0 * M_PI;
		}
	}

	usleep(dtMicroseconds);

	getSize = i * 2;
	return 0;
}

/* end */
