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

#include "HackRFSink.h"
#include "util.h"
#include "parsekv.h"
#include "UDPSource.h"

HackRFSink *HackRFSink::m_this = 0;
const std::vector<int> HackRFSink::m_vgains({0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60, 62});
const std::vector<int> HackRFSink::m_bwfilt({1750000, 2500000, 3500000, 5000000, 5500000, 6000000, 7000000,  8000000, 9000000, 10000000, 12000000, 14000000, 15000000, 20000000, 24000000, 28000000});

// Open HackRF device.
HackRFSink::HackRFSink(int dev_index) :
    m_dev(0),
    m_sampleRate(5000000),
    m_frequency(100000000),
	m_ppm(0),
    m_vgaGain(22),
    m_bandwidth(2500000),
    m_extAmp(false),
    m_biasAnt(false),
    m_running(false),
    m_thread(0),
    m_iqSamplesIndex(0)
{
    m_devname = "HackRFSink";

    hackrf_error rc = (hackrf_error) hackrf_init();

    if (rc != HACKRF_SUCCESS)
    {
        std::ostringstream err_ostr;
        err_ostr << "HackRFSink::HackRFSink: failed to open HackRF library (" << rc << ": " << hackrf_error_name(rc) << ")";
        m_error = err_ostr.str();
        m_dev = 0;
    }
    else
    {
        hackrf_device_list_t *hackrf_devices = hackrf_device_list();

        rc = (hackrf_error) hackrf_device_list_open(hackrf_devices, dev_index, &m_dev);

        if (rc != HACKRF_SUCCESS)
        {
            std::ostringstream err_ostr;
            err_ostr << "HackRFSink::HackRFSink: failed to open HackRF device " << dev_index << " (" << rc << ": " << hackrf_error_name(rc) << ")";
            m_error = err_ostr.str();
            m_dev = 0;
        }
    }

    std::ostringstream vgains_ostr;

    for (int g: m_vgains) {
        vgains_ostr << g << " ";
    }

    m_vgainsStr = vgains_ostr.str();

    std::ostringstream bwfilt_ostr;
    bwfilt_ostr << std::fixed << std::setprecision(2);

    for (int b: m_bwfilt) {
        bwfilt_ostr << b * 1e-6 << " ";
    }

    m_bwfiltStr = bwfilt_ostr.str();

    m_this = this;
}

HackRFSink::~HackRFSink()
{
    if (m_dev) {
        hackrf_close(m_dev);
    }

    hackrf_error rc = (hackrf_error) hackrf_exit();
    std::cerr << "HackRFSink::~HackRFSink: HackRF library exit: " << rc << ": " << hackrf_error_name(rc) << std::endl;

    m_this = 0;
}

void HackRFSink::get_device_names(std::vector<std::string>& devices)
{
    hackrf_device *hackrf_ptr;
    read_partid_serialno_t read_partid_serialno;
    hackrf_error rc;
    int i;

    rc = (hackrf_error) hackrf_init();

    if (rc != HACKRF_SUCCESS)
    {
        std::cerr << "HackRFSink::get_device_names: Failed to open HackRF library: " << rc << ": " << hackrf_error_name(rc) << std::endl;
        return;
    }

    hackrf_device_list_t *hackrf_devices = hackrf_device_list();

    devices.clear();

    for (i=0; i < hackrf_devices->devicecount; i++)
    {
        rc = (hackrf_error) hackrf_device_list_open(hackrf_devices, i, &hackrf_ptr);

        if (rc == HACKRF_SUCCESS)
        {
            std::cerr << "HackRFSink::get_device_names: try to get device " << i << " serial number" << std::endl;
            rc = (hackrf_error) hackrf_board_partid_serialno_read(hackrf_ptr, &read_partid_serialno);

            if (rc != HACKRF_SUCCESS)
            {
                std::cerr << "HackRFSink::get_device_names: failed to get device " << i << " serial number: " << rc << ": " << hackrf_error_name(rc) << std::endl;
                hackrf_close(hackrf_ptr);
                continue;
            }
            else
            {
                std::cerr << "HackRFSink::get_device_names: device " << i << " OK" << std::endl;
                hackrf_close(hackrf_ptr);
            }

            uint32_t serial_msb = read_partid_serialno.serial_no[2];
            uint32_t serial_lsb = read_partid_serialno.serial_no[3];
            std::ostringstream devname_ostr;

            devname_ostr << "Serial " << std::hex << std::setw(8) << std::setfill('0') << serial_msb << serial_lsb;
            devices.push_back(devname_ostr.str());
        }
        else
        {
            std::cerr << "HackRFSink::get_device_names: failed to open device " << i << std::endl;
        }
    }

    hackrf_device_list_free(hackrf_devices);
    rc = (hackrf_error) hackrf_exit();
    std::cerr << "HackRFSink::get_device_names: HackRF library exit: " << rc << ": " << hackrf_error_name(rc) << std::endl;
}

std::uint32_t HackRFSink::get_sample_rate()
{
    return m_sampleRate;
}

std::uint32_t HackRFSink::get_frequency()
{
    return m_frequency;
}

void HackRFSink::print_specific_parms()
{
	fprintf(stderr, "LO correction:     %.1f\n", m_ppm);
    fprintf(stderr, "VGA gain:          %d\n", m_vgaGain);
    fprintf(stderr, "Bandwidth          %d Hz\n", m_bandwidth);
    fprintf(stderr, "External Amp       %s\n", m_extAmp ? "enabled" : "disabled");
    fprintf(stderr, "Bias ant           %s\n", m_biasAnt ? "enabled" : "disabled");
}

bool HackRFSink::configure(uint32_t changeFlags,
        uint32_t sample_rate,
		uint64_t frequency,
        bool ext_amp,
        bool bias_ant,
        int vga_gain,
        uint32_t bandwidth
)
{
    hackrf_error rc;

    if (!m_dev) {
        return false;
    }

    if (changeFlags & 0x1)
    {
    	m_frequency = frequency;

        rc = (hackrf_error) hackrf_set_freq(m_dev, m_frequency);

        if (rc != HACKRF_SUCCESS)
        {
            std::ostringstream err_ostr;
            err_ostr << "HackRFSink::configure(flags): could not set center frequency to " << m_frequency << " Hz";
            m_error = err_ostr.str();
            return false;
        }
        else
        {
            std::cerr << "HackRFSink::configure(flags): set center frequency to " << m_frequency << " Hz" << std::endl;
        }
    }

    if (changeFlags & 0x2)
    {
        rc = (hackrf_error) hackrf_set_sample_rate_manual(m_dev, sample_rate, 1);

        if (rc != HACKRF_SUCCESS)
        {
            std::ostringstream err_ostr;
            err_ostr << "HackRFSink::configure(flags): could not set sample rate to " << sample_rate << " Hz";
            m_error = err_ostr.str();
            return false;
        }
        else
        {
            std::cerr << "HackRFSink::configure(flags): set sample rate to " << sample_rate << " Hz" << std::endl;
            m_sampleRate = sample_rate;
        }
    }

    if (changeFlags & 0x8)
    {
        m_vgaGain = vga_gain;

        rc = (hackrf_error) hackrf_set_vga_gain(m_dev, m_vgaGain);

        if (rc != HACKRF_SUCCESS)
        {
            std::ostringstream err_ostr;
            err_ostr << "HackRFSink::configure(flags): could not set VGA gain to " << m_vgaGain << " dB";
            m_error = err_ostr.str();
            return false;
        }
        else
        {
            std::cerr << "HackRFSink::configure(flags): set VGA gain to " << m_vgaGain << " dB" << std::endl;
        }
    }

    if (changeFlags & 0x10)
    {
        m_biasAnt = bias_ant;

        rc = (hackrf_error) hackrf_set_antenna_enable(m_dev, (m_biasAnt ? 1 : 0));

        if (rc != HACKRF_SUCCESS)
        {
            std::ostringstream err_ostr;
            err_ostr << "HackRFSink::configure(flags): could not set bias antenna to " << m_biasAnt;
            m_error = err_ostr.str();
            return false;
        }
        else
        {
            std::cerr << "HackRFSink::configure(flags): set bias antenna to " << m_biasAnt << std::endl;
        }
    }

    if (changeFlags & 0x20)
    {
        m_extAmp = ext_amp;

        rc = (hackrf_error) hackrf_set_amp_enable(m_dev, (m_extAmp ? 1 : 0));

        if (rc != HACKRF_SUCCESS)
        {
            std::ostringstream err_ostr;
            err_ostr << "HackRFSink::configure(flags): could not set extra amplifier to " << m_extAmp;
            m_error = err_ostr.str();
            return false;
        }
        else
        {
            std::cerr << "HackRFSink::configure(flags): set exra amplifier to " << m_extAmp << std::endl;
        }
    }

    if (changeFlags & 0x40)
    {
        m_bandwidth = bandwidth;
        uint32_t hackRFBandwidth = hackrf_compute_baseband_filter_bw_round_down_lt(m_bandwidth);
        rc = (hackrf_error) hackrf_set_baseband_filter_bandwidth(m_dev, hackRFBandwidth);

        if (rc != HACKRF_SUCCESS)
        {
            std::ostringstream err_ostr;
            err_ostr << "HackRFSink::configure(flags): could not set bandwidth to " << hackRFBandwidth << " Hz (" << m_bandwidth << " Hz requested)";
            m_error = err_ostr.str();
            return false;
        }
        else
        {
            std::cerr << "HackRFSink::configure(flags): set bandwidth to " << hackRFBandwidth << " Hz (" << m_bandwidth << " Hz requested)" << std::endl;
        }
    }

    return true;
}

bool HackRFSink::configure(parsekv::pairs_type& m)
{
    uint32_t sampleRate = 5000000;
    uint64_t frequency = m_confFreq;
    float ppm = m_ppm;
    int vgaGain = 22;
    uint32_t bandwidth = 2500000;
    bool extAmp = false;
    bool antBias = false;
    std::uint32_t changeFlags = 0;

	if (m.find("srate") != m.end())
	{
		std::cerr << "HackRFSink::configure: srate: " << m["srate"] << std::endl;
		sampleRate = atoi(m["srate"].c_str());

		if ((sampleRate < 1000000) || (sampleRate > 20000000))
		{
		    std::cerr << "HackRFSink::configure: Invalid sample rate " << sampleRate << " skipping" << std::endl;
		}
		else
		{
	        changeFlags |= 0x2;
		}
	}

	if (m.find("freq") != m.end())
	{
		std::cerr << "HackRFSink::configure: freq: " << m["freq"] << std::endl;
		frequency = strtoll(m["freq"].c_str(), 0, 10);

		if ((frequency < 1000000) || (frequency > 6000000000))
		{
            std::cerr << "HackRFSink::configure: Invalid frequency " << frequency << " skipping" << std::endl;
		}
		else
		{
	        changeFlags |= 0x1;
		}
	}

	if (m.find("vgain") != m.end())
	{
		std::cerr << "HackRFSink::configure: vgain: " << m["vgain"] << std::endl;
		vgaGain = atoi(m["vgain"].c_str());

		if (strcasecmp(m["vgain"].c_str(), "list") == 0)
		{
		    std::cerr << "HackRFSink::configure: Available VGA gains (dB): " << m_vgainsStr << std::endl;
		}
		else if (find(m_vgains.begin(), m_vgains.end(), vgaGain) == m_vgains.end())
		{
		    std::cerr << "HackRFSink::configure: VGA gain not supported. Available gains (dB): " << m_vgainsStr << std::endl;
		}
		else
		{
	        changeFlags |= 0x8;
		}
	}

	if (m.find("bwfilter") != m.end())
	{
		std::cerr << "HackRFSink::configure: bwfilter: " << m["bwfilter"] << std::endl;
		bandwidth = atoi(m["bwfilter"].c_str());

		if (strcasecmp(m["bwfilter"].c_str(), "list") == 0)
		{
		    std::cerr << "HackRFSink::configure: Available filter bandwidths (MHz): " << m_bwfiltStr << std::endl;
		}
		else
		{
	        double tmpbwd;

	        if (!parse_dbl(m["bwfilter"].c_str(), tmpbwd))
	        {
	            std::cerr << "HackRFSink::configure: Invalid filter bandwidth" << std::endl;
	        }
	        else
	        {
	            long int tmpbwi = lrint(tmpbwd * 1000000);

	            if (tmpbwi <= INT_MIN || tmpbwi >= INT_MAX)
	            {
	                std::cerr << "HackRFSink::configure: Invalid filter bandwidth" << std::endl;
	            }
	            else
	            {
	                bandwidth = tmpbwi;

	                if (find(m_bwfilt.begin(), m_bwfilt.end(), bandwidth) == m_bwfilt.end())
	                {
	                    std::cerr << "HackRFSink::configure: Filter bandwidth not supported. Available bandwidths (MHz): " << m_bwfiltStr << std::endl;
	                }
	                else
	                {
	                    changeFlags |= 0x40;

	                }
	            }
	        }
		}
	}

	if (m.find("extamp") != m.end())
	{
		std::cerr << "HackRFSink::configure: extamp: " << m["extamp"] << std::endl;
		extAmp = m["extamp"] == "1";
        changeFlags |= 0x20;
	}

	if (m.find("antbias") != m.end())
	{
		std::cerr << "HackRFSink::configure: antbias: " << m["antbias"] << std::endl;
		antBias = m["antbias"] == "1";
        changeFlags |= 0x10;
	}

    if (m.find("ppmp") != m.end())
	{
		std::cerr << "HackRFSink::configure: ppmp: " << m["ppmp"] << std::endl;
		errno = 0;
		char * e;
		ppm = std::strtod(m["ppmp"].c_str(), &e);

		if (*e == '\0' && errno == 0) // Conversion to float OK
		{
			m_ppm = ppm;
			changeFlags |= 0x1;
		}
	}
    else if (m.find("ppmn") != m.end())
	{
		std::cerr << "HackRFSink::configure: ppmn: " << m["ppmn"] << std::endl;
		errno = 0;
		char * e;
		ppm = std::strtod(m["ppmn"].c_str(), &e);

		if (*e == '\0' && errno == 0) // Conversion to float OK
		{
			m_ppm = -ppm;
			changeFlags |= 0x1;
		}
	}

	if (m.find("interp") != m.end())
	{
		std::cerr << "HackRFSink::configure: interp: " << m["interp"] << std::endl;
		int log2Interp = atoi(m["interp"].c_str());

		if ((log2Interp < 0) || (log2Interp > 6))
		{
		    std::cerr << "HackRFSink::configure: Invalid log2 interpolation factor" << std::endl;
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
	tuner_freq += tuner_freq * m_ppm * 1e-6;

    return configure(changeFlags, sampleRate, tuner_freq, extAmp, antBias, vgaGain, bandwidth);
}

bool HackRFSink::start(DataBuffer<IQSample> *buf, std::atomic_bool *stop_flag)
{
    m_buf = buf;
    m_stop_flag = stop_flag;

    if (m_thread == 0)
    {
        std::cerr << "HackRFSink::start: starting" << std::endl;
        m_running = true;
        m_thread = new std::thread(run, m_dev, stop_flag);
        sleep(1);
        return *this;
    }
    else
    {
        std::cerr << "HackRFSink::start: error" << std::endl;
        m_error = "Sink thread already started";
        return false;
    }
}

void HackRFSink::run(hackrf_device* dev, std::atomic_bool *stop_flag)
{
    std::cerr << "HackRFSink::run" << std::endl;
    void *msgBuf = 0;
    char msgBufSend[128];

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

    hackrf_error rc = (hackrf_error) hackrf_start_tx(dev, tx_callback, 0);

    if (rc == HACKRF_SUCCESS)
    {
        while (!stop_flag->load() && (hackrf_is_streaming(dev) == HACKRF_TRUE))
        {
            sleep(1);

            int len = nn_recv(m_this->m_nnReceiver, &msgBuf, NN_MSG, NN_DONTWAIT);

            if ((len > 0) && msgBuf)
            {
                std::string msg((char *) msgBuf, len);
                std::cerr << "HackRFSink::run: received message: " << msg << std::endl;
                bool success = m_this->DeviceSink::configure(msg);
                nn_freemsg(msgBuf);
                msgBuf = 0;
                if (!success) {
                    std::cerr << "HackRFSink::run: config error: " << m_this->DeviceSink::error() << std::endl;
                }
            }

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
        }

        std::cerr << "HackRFSink::run: finished" << std::endl;

        rc = (hackrf_error) hackrf_stop_tx(dev);

        if (rc != HACKRF_SUCCESS)
        {
            std::cerr << "HackRFSink::run: Cannot stop HackRF Tx: " << rc << ": " << hackrf_error_name(rc) << std::endl;
        }
    }
    else
    {
        std::cerr << "HackRFSink::run: Cannot start HackRF Tx: " << rc << ": " << hackrf_error_name(rc) << std::endl;
    }
}

bool HackRFSink::stop()
{
    std::cerr << "HackRFSink::stop" << std::endl;

    m_thread->join();
    delete m_thread;
    return true;
}

int HackRFSink::tx_callback(hackrf_transfer* transfer)
{
    int bytes_to_read = transfer->valid_length; // bytes to read from FIFO as expected by the Tx

    if (m_this)
    {
        m_this->callback((char *) transfer->buffer, bytes_to_read);
    }

    return 0;
}

void HackRFSink::callback(char* buf, int len)
{
    int i = 0;

    for (; i < len/2; i++)
    {
        if (m_iqSamplesIndex == m_iqSamples.size())
        {
            if (m_buf->test_buffer_fill((len/2) - i))
            {
                m_iqSamples = m_buf->pull();
//                fprintf(stderr, "HackRFSink::callback: len: %d, pull size: %lu, queue size: %lu\n", len, m_iqSamples.size(), m_buf->queued_vectors());
                m_iqSamplesIndex = 0;
            }
            else
            {
                m_iqSamples.clear();
                m_iqSamplesIndex = 0;
                break;
            }
        }

        buf[2*i]     = m_iqSamples[m_iqSamplesIndex].real() >> 8;
        buf[2*i+1]   = m_iqSamples[m_iqSamplesIndex].imag() >> 8;
        m_iqSamplesIndex++;
    }

    for (; i < len/2; i++)
    {
        buf[2*i]     = 8;
        buf[2*i+1]   = 0;
    }
}
