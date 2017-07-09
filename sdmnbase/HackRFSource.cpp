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

#include <cstring>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <thread>
#include <cstdlib>

#include "HackRFSource.h"
#include "util.h"
#include "parsekv.h"

HackRFSource *HackRFSource::m_this = 0;
const std::vector<int> HackRFSource::m_lgains({0, 8, 16, 24, 32, 40});
const std::vector<int> HackRFSource::m_vgains({0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60, 62});
const std::vector<int> HackRFSource::m_bwfilt({1750000, 2500000, 3500000, 5000000, 5500000, 6000000, 7000000,  8000000, 9000000, 10000000, 12000000, 14000000, 15000000, 20000000, 24000000, 28000000});

// Open HackRF device.
HackRFSource::HackRFSource(int dev_index) :
    m_dev(0),
    m_sampleRate(5000000),
    m_frequency(100000000),
	m_ppm(0),
    m_lnaGain(16),
    m_vgaGain(22),
    m_bandwidth(2500000),
    m_extAmp(false),
    m_biasAnt(false),
    m_running(false),
    m_thread(0)
{
    hackrf_error rc = (hackrf_error) hackrf_init();

    if (rc != HACKRF_SUCCESS)
    {
        std::ostringstream err_ostr;
        err_ostr << "Failed to open HackRF library (" << rc << ": " << hackrf_error_name(rc) << ")";
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
            err_ostr << "Failed to open HackRF device " << dev_index << " (" << rc << ": " << hackrf_error_name(rc) << ")";
            m_error = err_ostr.str();
            m_dev = 0;
        }
    }

    std::ostringstream lgains_ostr;

    for (int g: m_lgains) {
        lgains_ostr << g << " ";
    }

    m_lgainsStr = lgains_ostr.str();

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

HackRFSource::~HackRFSource()
{
    if (m_dev) {
        hackrf_close(m_dev);
    }

    hackrf_error rc = (hackrf_error) hackrf_exit();
    std::cerr << "HackRFSource::~HackRFSource: HackRF library exit: " << rc << ": " << hackrf_error_name(rc) << std::endl;

    m_this = 0;
}

void HackRFSource::get_device_names(std::vector<std::string>& devices)
{
    hackrf_device *hackrf_ptr;
    read_partid_serialno_t read_partid_serialno;
    hackrf_error rc;
    int i;

    rc = (hackrf_error) hackrf_init();

    if (rc != HACKRF_SUCCESS)
    {
        std::cerr << "HackRFSource::get_device_names: Failed to open HackRF library: " << rc << ": " << hackrf_error_name(rc) << std::endl;
        return;
    }

    hackrf_device_list_t *hackrf_devices = hackrf_device_list();

    devices.clear();

    for (i=0; i < hackrf_devices->devicecount; i++)
    {
        rc = (hackrf_error) hackrf_device_list_open(hackrf_devices, i, &hackrf_ptr);

        if (rc == HACKRF_SUCCESS)
        {
            std::cerr << "HackRFSource::get_device_names: try to get device " << i << " serial number" << std::endl;
            rc = (hackrf_error) hackrf_board_partid_serialno_read(hackrf_ptr, &read_partid_serialno);

            if (rc != HACKRF_SUCCESS)
            {
                std::cerr << "HackRFSource::get_device_names: failed to get device " << i << " serial number: " << rc << ": " << hackrf_error_name(rc) << std::endl;
                hackrf_close(hackrf_ptr);
                continue;
            }
            else
            {
                std::cerr << "HackRFSource::get_device_names: device " << i << " OK" << std::endl;
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
            std::cerr << "HackRFSource::get_device_names: failed to open device " << i << std::endl;
        }
    }

    hackrf_device_list_free(hackrf_devices);
    rc = (hackrf_error) hackrf_exit();
    std::cerr << "HackRFSource::get_device_names: HackRF library exit: " << rc << ": " << hackrf_error_name(rc) << std::endl;
}

std::uint32_t HackRFSource::get_sample_rate()
{
    return m_sampleRate;
}

std::uint32_t HackRFSource::get_frequency()
{
    return m_frequency;
}

void HackRFSource::print_specific_parms()
{
	fprintf(stderr, "LO correction:     %.1f\n", m_ppm);
    fprintf(stderr, "LNA gain:          %d\n", m_lnaGain);
    fprintf(stderr, "VGA gain:          %d\n", m_vgaGain);
    fprintf(stderr, "Bandwidth          %d Hz\n", m_bandwidth);
    fprintf(stderr, "External Amp       %s\n", m_extAmp ? "enabled" : "disabled");
    fprintf(stderr, "Bias ant           %s\n", m_biasAnt ? "enabled" : "disabled");
}

bool HackRFSource::configure(uint32_t changeFlags,
        uint32_t sample_rate,
		uint64_t frequency,
        bool ext_amp,
        bool bias_ant,
        int lna_gain,
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
            err_ostr << "Could not set center frequency to " << m_frequency << " Hz";
            m_error = err_ostr.str();
            std::cerr << "HackRFSource::configure(flags): " + m_error << std::endl;
            return false;
        }
        else
        {
            std::cerr << "HackRFSource::configure(flags): center frequency set to: " << m_frequency << " Hz" << std::endl;
        }
    }

    if (changeFlags & 0x2)
    {
        m_sampleRate = sample_rate;

        rc = (hackrf_error) hackrf_set_sample_rate_manual(m_dev, m_sampleRate, 1);

        if (rc != HACKRF_SUCCESS)
        {
            std::ostringstream err_ostr;
            err_ostr << "Could not set sample rate to " << m_sampleRate << " Hz";
            m_error = err_ostr.str();
            std::cerr << "HackRFSource::configure(flags): " + m_error << std::endl;
            return false;
        }
        else
        {
            std::cerr << "HackRFSource::configure(flags): sample rate set to: " << m_sampleRate << " Hz" << std::endl;
        }
    }

    if (changeFlags & 0x4)
    {
        m_lnaGain = lna_gain;

        rc = (hackrf_error) hackrf_set_lna_gain(m_dev, m_lnaGain);

        if (rc != HACKRF_SUCCESS)
        {
            std::ostringstream err_ostr;
            err_ostr << "Could not set LNA gain to " << m_lnaGain << " dB";
            m_error = err_ostr.str();
            std::cerr << "HackRFSource::configure(flags): " + m_error << std::endl;
            return false;
        }
        else
        {
            std::cerr << "HackRFSource::configure(flags): LNA gain set to: " << m_lnaGain << " dB" << std::endl;
        }
    }

    if (changeFlags & 0x8)
    {
        m_vgaGain = vga_gain;

        rc = (hackrf_error) hackrf_set_vga_gain(m_dev, m_vgaGain);

        if (rc != HACKRF_SUCCESS)
        {
            std::ostringstream err_ostr;
            err_ostr << "Could not set VGA gain to " << m_vgaGain << " dB";
            m_error = err_ostr.str();
            std::cerr << "HackRFSource::configure(flags): " + m_error << std::endl;
            return false;
        }
        else
        {
            std::cerr << "HackRFSource::configure(flags): VGA gain set to: " << m_vgaGain << " dB" << std::endl;
        }
    }

    if (changeFlags & 0x10)
    {
        m_biasAnt = bias_ant;

        rc = (hackrf_error) hackrf_set_antenna_enable(m_dev, (m_biasAnt ? 1 : 0));

        if (rc != HACKRF_SUCCESS)
        {
            std::ostringstream err_ostr;
            err_ostr << "Could not set bias antenna to " << m_biasAnt;
            m_error = err_ostr.str();
            std::cerr << "HackRFSource::configure(flags): " + m_error << std::endl;
            return false;
        }
        else
        {
            std::cerr << "HackRFSource::configure(flags): bias antenna set to: " << m_biasAnt << std::endl;
        }
    }

    if (changeFlags & 0x20)
    {
        m_extAmp = ext_amp;

        rc = (hackrf_error) hackrf_set_amp_enable(m_dev, (m_extAmp ? 1 : 0));

        if (rc != HACKRF_SUCCESS)
        {
            std::ostringstream err_ostr;
            err_ostr << "Could not set extra amplifier to " << m_extAmp;
            m_error = err_ostr.str();
            std::cerr << "HackRFSource::configure(flags): " + m_error << std::endl;
            return false;
        }
        else
        {
            std::cerr << "HackRFSource::configure(flags): extra amplifier set to: " << m_extAmp << std::endl;
        }
    }

    if (changeFlags & 0x40)
    {
        m_bandwidth = bandwidth;
        uint32_t hackRFBandwidth = hackrf_compute_baseband_filter_bw_round_down_lt(m_bandwidth + 1); // +1 so the round down to lower than yields desired bandwidth
        rc = (hackrf_error) hackrf_set_baseband_filter_bandwidth(m_dev, hackRFBandwidth);

        if (rc != HACKRF_SUCCESS)
        {
            std::ostringstream err_ostr;
            err_ostr << "Could not set bandwidth to " << hackRFBandwidth << " Hz (" << m_bandwidth << " Hz requested)";
            m_error = err_ostr.str();
            std::cerr << "HackRFSource::configure(flags): " + m_error << std::endl;
            return false;
        }
        else
        {
            std::cerr << "HackRFSource::configure(flags): bandwidth set to: " << hackRFBandwidth << " Hz" << std::endl;
        }
    }

    return true;
}

bool HackRFSource::configure(parsekv::pairs_type& m)
{
    uint32_t sampleRate = 5000000;
    uint64_t frequency = m_confFreq;
    float ppm = m_ppm;
    int lnaGain = 16;
    int vgaGain = 22;
    uint32_t bandwidth = 2500000;
    bool extAmp = false;
    bool antBias = false;
    int fcpos = 2; // default centered
    std::uint32_t changeFlags = 0;

	if (m.find("srate") != m.end())
	{
		std::cerr << "HackRFSource::configure: srate: " << m["srate"] << std::endl;
		sampleRate = atoi(m["srate"].c_str());

		if ((sampleRate < 1000000) || (sampleRate > 20000000))
		{
			m_error = "Invalid sample rate";
			std::cerr << "HackRFSource::configure: " + m_error << std::endl;
			return false;
		}

        changeFlags |= 0x2;

        if (m_fcPos != 2)
        {
            changeFlags |= 0x1; // need to adjust actual center frequency if not centered
        }
	}

	if (m.find("freq") != m.end())
	{
		std::cerr << "HackRFSource::configure: freq: " << m["freq"] << std::endl;
		frequency = strtoll(m["freq"].c_str(), 0, 10);

		if ((frequency < 1000000) || (frequency > 6000000000))
		{
			m_error = "Invalid frequency";
            std::cerr << "HackRFSource::configure: " + m_error << std::endl;
			return false;
		}

        changeFlags |= 0x1;
	}

	if (m.find("lgain") != m.end())
	{
		std::cerr << "HackRFSource::configure: lgain: " << m["lgain"] << std::endl;

		if (strcasecmp(m["lgain"].c_str(), "list") == 0)
		{
			m_error = "Available LNA gains (dB): " + m_lgainsStr;
            std::cerr << "HackRFSource::configure: " + m_error << std::endl;
			return false;
		}

		lnaGain = atoi(m["lgain"].c_str());

		if (find(m_lgains.begin(), m_lgains.end(), lnaGain) == m_lgains.end())
		{
			m_error = "LNA gain not supported. Available gains (dB): " + m_lgainsStr;
			std::cerr << "HackRFSource::configure: " + m_error << std::endl;
			return false;
		}

        changeFlags |= 0x4;
	}

	if (m.find("vgain") != m.end())
	{
		std::cerr << "HackRFSource::configure: vgain: " << m["vgain"] << std::endl;
		vgaGain = atoi(m["vgain"].c_str());

		if (strcasecmp(m["vgain"].c_str(), "list") == 0)
		{
			m_error = "Available VGA gains (dB): " + m_vgainsStr;
            std::cerr << "HackRFSource::configure: " + m_error << std::endl;
			return false;
		}

		if (find(m_vgains.begin(), m_vgains.end(), vgaGain) == m_vgains.end())
		{
			m_error = "VGA gain not supported. Available gains (dB): " + m_vgainsStr;
            std::cerr << "HackRFSource::configure: " + m_error << std::endl;
			return false;
		}

        changeFlags |= 0x8;
	}

	if (m.find("bwfilter") != m.end())
	{
		std::cerr << "HackRFSource::configure: bwfilter: " << m["bwfilter"] << std::endl;
		bandwidth = atoi(m["bwfilter"].c_str());

		if (strcasecmp(m["bwfilter"].c_str(), "list") == 0)
		{
			m_error = "Available filter bandwidths (MHz): " + m_bwfiltStr;
            std::cerr << "HackRFSource::configure: " + m_error << std::endl;
			return false;
		}

		double tmpbwd;

		if (!parse_dbl(m["bwfilter"].c_str(), tmpbwd))
		{
			m_error = "Invalid filter bandwidth";
            std::cerr << "HackRFSource::configure: " + m_error << std::endl;
			return false;
		}
		else
		{
			long int tmpbwi = lrint(tmpbwd * 1000000);

			if (tmpbwi <= INT_MIN || tmpbwi >= INT_MAX) {
				m_error = "Invalid filter bandwidth";
	            std::cerr << "HackRFSource::configure: " + m_error << std::endl;
				return false;
			}
			else
			{
				bandwidth = tmpbwi;

				if (find(m_bwfilt.begin(), m_bwfilt.end(), bandwidth) == m_bwfilt.end())
				{
					m_error = "Filter bandwidth not supported. Available bandwidths (MHz): " + m_bwfiltStr;
		            std::cerr << "HackRFSource::configure: " + m_error << std::endl;
					return false;
				}
			}
		}

        changeFlags |= 0x40;
	}

	if (m.find("extamp") != m.end())
	{
		std::cerr << "HackRFSource::configure: extamp: " << m["extamp"] << std::endl;
        extAmp = m["extamp"] == "1";
        changeFlags |= 0x20;
	}

	if (m.find("antbias") != m.end())
	{
		std::cerr << "HackRFSource::configure: antbias: " << m["antbias"] << std::endl;
        antBias = m["antbias"] == "1";
        changeFlags |= 0x10;
	}

    if (m.find("ppmp") != m.end())
	{
		std::cerr << "HackRFSource::configure: ppmp: " << m["ppmp"] << std::endl;
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
		std::cerr << "HackRFSource::configure: ppmn: " << m["ppmn"] << std::endl;
		errno = 0;
		char * e;
		ppm = std::strtod(m["ppmn"].c_str(), &e);

		if (*e == '\0' && errno == 0) // Conversion to float OK
		{
			m_ppm = -ppm;
			changeFlags |= 0x1;
		}
	}

	if (m.find("fcpos") != m.end())
	{
		std::cerr << "HackRFSource::configure: fcpos: " << m["fcpos"] << std::endl;
		fcpos = atoi(m["fcpos"].c_str());

		if ((fcpos < 0) || (fcpos > 2))
		{
			m_error = "Invalid center frequency position";
            std::cerr << "HackRFSource::configure: " + m_error << std::endl;
			return false;
		}
		else
		{
			m_fcPos = fcpos;
		}

        changeFlags |= 0x1; // need to adjust actual center frequency if not centered
	}

	if (m.find("decim") != m.end())
	{
		std::cerr << "HackRFSource::configure: decim: " << m["decim"] << std::endl;
		int log2Decim = atoi(m["decim"].c_str());

		if ((log2Decim < 0) || (log2Decim > 6))
		{
			m_error = "Invalid log2 decimation factor";
            std::cerr << "HackRFSource::configure: " + m_error << std::endl;
			return false;
		}
		else
		{
			m_decim = log2Decim;
		}
	}

    m_confFreq = frequency;
	double tuner_freq;

	if (m_fcPos == 0) { // Infradyne
		tuner_freq = frequency + 0.25 * sampleRate;
	} else if (m_fcPos == 1) { // Supradyne
		tuner_freq = frequency - 0.25 * sampleRate;
	} else { // Centered
		tuner_freq = frequency;
	}

	tuner_freq += tuner_freq * m_ppm * 1e-6;

    return configure(changeFlags, sampleRate, tuner_freq, extAmp, antBias, lnaGain, vgaGain, bandwidth);
}

bool HackRFSource::start(DataBuffer<IQSample> *buf, std::atomic_bool *stop_flag)
{
    m_buf = buf;
    m_stop_flag = stop_flag;

    if (m_thread == 0)
    {
        std::cerr << "HackRFSource::start: starting" << std::endl;
        m_running = true;
        m_thread = new std::thread(run, m_dev, stop_flag);
        sleep(1);
        return *this;
    }
    else
    {
        std::cerr << "HackRFSource::start: error" << std::endl;
        m_error = "Source thread already started";
        return false;
    }
}

void HackRFSource::run(hackrf_device* dev, std::atomic_bool *stop_flag)
{
    std::cerr << "HackRFSource::run" << std::endl;
    void *msgBuf = 0;

    hackrf_error rc = (hackrf_error) hackrf_start_rx(dev, rx_callback, 0);

    if (rc == HACKRF_SUCCESS)
    {
        while (!stop_flag->load() && (hackrf_is_streaming(dev) == HACKRF_TRUE))
        {
            sleep(1);

            int len = nn_recv(m_this->m_nnReceiver, &msgBuf, NN_MSG, NN_DONTWAIT);

            if ((len > 0) && msgBuf)
            {
                std::string msg((char *) msgBuf, len);
                std::cerr << "HackRFSource::run: received: " << msg << std::endl;
                m_this->DeviceSource::configure(msg);
                nn_freemsg(msgBuf);
                msgBuf = 0;
            }
        }

        rc = (hackrf_error) hackrf_stop_rx(dev);

        if (rc != HACKRF_SUCCESS)
        {
            std::cerr << "HackRFSource::run: Cannot stop HackRF Rx: " << rc << ": " << hackrf_error_name(rc) << std::endl;
        }
    }
    else
    {
        std::cerr << "HackRFSource::run: Cannot start HackRF Rx: " << rc << ": " << hackrf_error_name(rc) << std::endl;
    }
}

bool HackRFSource::stop()
{
    std::cerr << "HackRFSource::stop" << std::endl;

    m_thread->join();
    delete m_thread;
    return true;
}

int HackRFSource::rx_callback(hackrf_transfer* transfer)
{
    int bytes_to_write = transfer->valid_length;

    if (m_this)
    {
        m_this->callback((signed char *) transfer->buffer, bytes_to_write);
    }

    return 0;
}

void HackRFSource::callback(const signed char* buf, int len)
{
    IQSampleVector iqsamples;

    iqsamples.resize(len/2);

    for (int i = 0; i < len/2; i++)
    {
        iqsamples[i]   = IQSample(buf[2*i],   buf[2*i+1]);
//        iqsamples[2*i+1] = IQSample(buf[4*i+2], buf[4*i+3]);
    }

    m_buf->push(move(iqsamples));
}
