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

#include <cstdlib>
#include <cstdio>
#include <climits>
#include <cmath>
#include <csignal>
#include <cstring>
#include <algorithm>
#include <atomic>
#include <memory>
#include <thread>
#include <unistd.h>
#include <getopt.h>
#include <sys/time.h>

#include "util.h"
#include "parsekv.h"
#include "DataBuffer.h"
#include "Downsampler.h"
#include "UDPSink.h"
#include "MovingAverage.h"

#include "RtlSdrSource.h"
#include "HackRFSource.h"
#include "AirspySource.h"
#include "BladeRFSource.h"
#include "include/SDRDaemon.h"

#define UDPSIZE 512

/** Flag is set on SIGINT / SIGTERM. */
static std::atomic_bool stop_flag(false);


/** Simple linear gain adjustment. */
void adjust_gain(SampleVector& samples, double gain)
{
    for (unsigned int i = 0, n = samples.size(); i < n; i++) {
        samples[i] *= gain;
    }
}

/**
 * Get data from output buffer and write to output stream.
 *
 * This code runs in a separate thread.
 */
void write_output_data(UDPSink *output,
		DataBuffer<IQSample> *buf,
        std::size_t buf_minfill)
{
    while (!stop_flag.load())
    {
        if (buf->queued_samples() == 0)
        {
            // The buffer is empty. Perhaps the output stream is consuming
            // samples faster than we can produce them. Wait until the buffer
            // is back at its nominal level to make sure this does not happen
            // too often.
            buf->wait_buffer_fill(buf_minfill);
        }

        if (buf->pull_end_reached())
        {
            // Reached end of stream.
            break;
        }

        // Get samples from buffer and write to output.
        IQSampleVector samples = buf->pull();
        output->write(samples);

        if (!(*output))
        {
            fprintf(stderr, "ERROR: Output: %s\n", output->error().c_str());
        }
    }
}


/** Handle Ctrl-C and SIGTERM. */
static void handle_sigterm(int sig)
{
    stop_flag.store(true);

    std::string msg = "\nGot signal ";
    msg += strsignal(sig);
    msg += ", stopping ...\n";

    const char *s = msg.c_str();
    write(STDERR_FILENO, s, strlen(s));
}


void usage()
{
    fprintf(stderr,
    "Usage: sdrdaemon [options]\n"
            "  -t devtype     Device type:\n"
            "                   - rtlsdr: RTL-SDR devices\n"
            "                   - hackrf: HackRF One or Jawbreaker\n"
            "                   - airspy: Airspy\n"
            "                   - bladerf: BladeRF\n"
            "  -c config      Startup configuration. Comma separated key=value configuration pairs\n"
    		"                 or just key for switches. See below for valid values\n"
            "  -d devidx      Device index, 'list' to show device list (default 0)\n"
            "  -b blocks      Set buffer size in number of UDP blocks (default: 480 512 samples blocks)\n"
            "  -I address     IP address. Samples are sent to this address (default: 127.0.0.1)\n"
            "  -D port        Data port. Samples are sent on this UDP port (default 9090)\n"
            "  -C port        Configuration port (default 9091, future). The configuration string as described below\n"
    		"                 is sent on this port via UDP to control the device\n"
            "\n"
    		"Configuration options for the decimator:\n"
            "  decim=<int>    log2 of decimation factor (default 0: no decimation)\n"
            "  fcpos=<int>    Center frequency position (default 2: center):\n"
            "                   - 0: Infradyne\n"
            "                   - 1: Supradyne\n"
            "                   - 2: Centered\n"
            "\n"
            "Configuration options for RTL-SDR devices\n"
            "  freq=<int>     Frequency of radio station in Hz (default 100000000)\n"
    		"                 valid values: 10M to 2.2G (working range depends on device)\n"
            "  srate=<int>    IF sample rate in Hz (default 1000000)\n"
            "                 (valid ranges: [225001, 300000], [900001, 3200000]))\n"
            "  gain=<float>   Set LNA gain in dB, or 'auto',\n"
            "                 or 'list' to just get a list of valid values (default auto)\n"
            "  blklen=<int>   Set read buffer size in seconds (default RTL-SDR default)\n"
            "  ppm=<int>      Set LO correction in PPM (default 0)\n"
            "  agc            Enable RTL AGC mode (default disabled)\n"
            "\n"
            "Configuration options for HackRF devices\n"
            "  freq=<int>     Frequency of radio station in Hz (default 100000000)\n"
    		"                 valid values: 1M to 6G\n"
            "  srate=<int>    IF sample rate in Hz (default 5000000)\n"
            "                 (valid ranges: [2500000,20000000]))\n"
            "  lgain=<int>    LNA gain in dB. 'list' to just get a list of valid values: (default 16)\n"
            "  vgain=<int>    VGA gain in dB. 'list' to just get a list of valid values: (default 22)\n"
            "  bwfilter=<int> Filter bandwidth in MHz. 'list' to just get a list of valid values: (default 2.5)\n"
            "  extamp         Enable extra RF amplifier (default disabled)\n"
            "  antbias        Enable antemma bias (default disabled)\n"
            "\n"
            "Configuration options for Airspy devices\n"
            "  freq=<int>     Frequency of radio station in Hz (default 100000000)\n"
    		"                 valid values: 24M to 1.8G\n"
            "  srate=<int>    IF sample rate in Hz. Depends on Airspy firmware and libairspy support\n"
    		"                 Airspy firmware and library must support dynamic sample rate query. (default 10000000)\n"
            "  lgain=<int>    LNA gain in dB. 'list' to just get a list of valid values: (default 8)\n"
            "  mgain=<int>    Mixer gain in dB. 'list' to just get a list of valid values: (default 8)\n"
            "  vgain=<int>    VGA gain in dB. 'list' to just get a list of valid values: (default 8)\n"
            "  antbias        Enable antemma bias (default disabled)\n"
            "  lagc           Enable LNA AGC (default disabled)\n"
            "  magc           Enable mixer AGC (default disabled)\n"
            "\n"
            "Configuration options for BladeRF devices\n"
            "  freq=<int>     Frequency of radio station in Hz (default 300000000)\n"
    		"                 valid values (with XB200): 100k to 3.8G\n"
    		"                 valid values (without XB200): 300M to 3.8G\n"
            "  srate=<int>    IF sample rate in Hz. Valid values: 48k to 40M (default 1000000)\n"
            "  bw=<int>       Bandwidth in Hz. 'list' to just get a list of valid values: (default 1500000)\n"
            "  lgain=<int>    LNA gain in dB. 'list' to just get a list of valid values: (default 3)\n"
            "  v1gain=<int>   VGA1 gain in dB. 'list' to just get a list of valid values: (default 20)\n"
            "  v2gain=<int>   VGA2 gain in dB. 'list' to just get a list of valid values: (default 9)\n"
            "\n");
}


void badarg(const char *label)
{
    usage();
    fprintf(stderr, "ERROR: Invalid argument for %s\n", label);
    exit(1);
}


bool parse_int(const char *s, int& v, bool allow_unit=false)
{
    char *endp;
    long t = strtol(s, &endp, 10);
    if (endp == s)
        return false;
    if ( allow_unit && *endp == 'k' &&
         t > INT_MIN / 1000 && t < INT_MAX / 1000 ) {
        t *= 1000;
        endp++;
    }
    if (*endp != '\0' || t < INT_MIN || t > INT_MAX)
        return false;
    v = t;
    return true;
}


/** Return Unix time stamp in seconds. */
double get_time()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + 1.0e-6 * tv.tv_usec;
}

static bool get_device(std::vector<std::string> &devnames, std::string& devtype, Source **srcsdr, int devidx)
{
    if (strcasecmp(devtype.c_str(), "rtlsdr") == 0)
    {
        RtlSdrSource::get_device_names(devnames);
    }
    else if (strcasecmp(devtype.c_str(), "hackrf") == 0)
    {
        HackRFSource::get_device_names(devnames);
    }
    else if (strcasecmp(devtype.c_str(), "airspy") == 0)
    {
        AirspySource::get_device_names(devnames);
    }
    else if (strcasecmp(devtype.c_str(), "bladerf") == 0)
    {
        BladeRFSource::get_device_names(devnames);
    }
    else
    {
        fprintf(stderr, "ERROR: wrong device type (-t option) must be one of the following:\n");
        fprintf(stderr, "       rtlsdr, hackrf, airspy, bladerf\n");
        return false;
    }

    if (devidx < 0 || (unsigned int)devidx >= devnames.size())
    {
        if (devidx != -1)
        {
            fprintf(stderr, "ERROR: invalid device index %d\n", devidx);
        }

        fprintf(stderr, "Found %u devices:\n", (unsigned int)devnames.size());

        for (unsigned int i = 0; i < devnames.size(); i++)
        {
            fprintf(stderr, "%2u: %s\n", i, devnames[i].c_str());
        }

        return false;
    }

    fprintf(stderr, "using device %d: %s\n", devidx, devnames[devidx].c_str());

    if (strcasecmp(devtype.c_str(), "rtlsdr") == 0)
    {
        // Open RTL-SDR device.
        *srcsdr = new RtlSdrSource(devidx);
    }
    else if (strcasecmp(devtype.c_str(), "hackrf") == 0)
    {
        // Open HackRF device.
        *srcsdr = new HackRFSource(devidx);
    }
    else if (strcasecmp(devtype.c_str(), "airspy") == 0)
    {
        // Open Airspy device.
        *srcsdr = new AirspySource(devidx);
    }
    else if (strcasecmp(devtype.c_str(), "bladerf") == 0)
    {
        // Open BladeRF device.
        *srcsdr = new BladeRFSource(devnames[devidx].c_str());
    }

    return true;
}

int main(int argc, char **argv)
{
    int     devidx  = 0;
    std::string  filename;
    std::string  alsadev("default");
    double  bufsecs = -1;
    std::string config_str;
    std::string devtype_str;
    std::vector<std::string> devnames;
    std::string dataaddress("127.0.0.1");
    unsigned int dataport = 9090;
    unsigned int cfgport = 9091;
    Source  *srcsdr = 0;
    unsigned int outputbuf_samples = 48 * UDPSIZE;

    fprintf(stderr,
            "SDRDaemon - Collect samples from SDR device and send it over the network via UDP\n");

    const struct option longopts[] = {
        { "devtype",    2, NULL, 't' },
        { "config",     2, NULL, 'c' },
        { "dev",        1, NULL, 'd' },
        { "buffer",     1, NULL, 'b' },
        { "daddress",   2, NULL, 'I' },
        { "dport",      1, NULL, 'D' },
        { "cport",      1, NULL, 'C' },
        { NULL,         0, NULL, 0 } };

    int c, longindex, value;
    while ((c = getopt_long(argc, argv,
    		"t:c:d:b:I:D:C:",
			longopts, &longindex)) >= 0)
    {
        switch (c)
        {
            case 't':
                devtype_str.assign(optarg);
                break;
            case 'c':
                config_str.assign(optarg);
                break;
            case 'd':
                if (!parse_int(optarg, devidx))
                    devidx = -1;
                break;
            case 'b':
                if (!parse_int(optarg, value) || (value < 0)) {
                    badarg("-b");
                } else {
                	outputbuf_samples = value;
                }
                break;
            case 'I':
                dataaddress.assign(optarg);
                break;
            case 'D':
                if (!parse_int(optarg, value) || (value < 0)) {
                    badarg("-D");
                } else {
               		dataport = value;
                }
                break;
            case 'C':
                if (!parse_int(optarg, value) || (value < 0)) {
                    badarg("-C");
                } else {
               		cfgport = value;
                }
                break;
            default:
                usage();
                fprintf(stderr, "ERROR: Invalid command line options\n");
                exit(1);
        }
    }

    if (optind < argc)
    {
        usage();
        fprintf(stderr, "ERROR: Unexpected command line options\n");
        exit(1);
    }

    // Catch Ctrl-C and SIGTERM
    struct sigaction sigact;
    sigact.sa_handler = handle_sigterm;
    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags = SA_RESETHAND;

    if (sigaction(SIGINT, &sigact, NULL) < 0)
    {
        fprintf(stderr, "WARNING: can not install SIGINT handler (%s)\n", strerror(errno));
    }

    if (sigaction(SIGTERM, &sigact, NULL) < 0)
    {
        fprintf(stderr, "WARNING: can not install SIGTERM handler (%s)\n", strerror(errno));
    }

    // Prepare output writer.
    UDPSink *udp_output_instance = new UDPSink(dataaddress, dataport, UDPSIZE);
    std::unique_ptr<UDPSink> udp_output(udp_output_instance);

    if (!(*udp_output))
    {
        fprintf(stderr, "ERROR: UDP Output: %s\n", udp_output->error().c_str());
        exit(1);
    }

    if (!get_device(devnames, devtype_str, &srcsdr, devidx))
    {
        exit(1);
    }

    if (!(*srcsdr))
    {
        fprintf(stderr, "ERROR source: %s\n", srcsdr->error().c_str());
        delete srcsdr;
        exit(1);
    }

    // Configure device and start streaming.

    namespace qi = boost::spirit::qi;
    parsekv::key_value_sequence<std::string::iterator> p;
    parsekv::pairs_type m;

    if (!qi::parse(config_str.begin(), config_str.end(), p, m))
    {
    	fprintf(stderr, "Configuration parsing failed\n");
    	delete srcsdr;
        return false;
    }

    if (!srcsdr->configure(m))
    {
        fprintf(stderr, "ERROR: source configuration: %s\n", srcsdr->error().c_str());
        delete srcsdr;
        exit(1);
    }

    // Prepare downsampler.
    Downsampler dn;

    if (!dn.configure(m))
    {
        fprintf(stderr, "ERROR: downsampler configuration: %s\n", dn.error().c_str());
        delete srcsdr;
        exit(1);
    }

    double freq = srcsdr->get_configured_frequency();
    fprintf(stderr, "tuned for:         %.6f MHz\n", freq * 1.0e-6);

    double tuner_freq = srcsdr->get_frequency();
    fprintf(stderr, "device tuned for:  %.6f MHz\n", tuner_freq * 1.0e-6);

    double ifrate = srcsdr->get_sample_rate();
    fprintf(stderr, "IF sample rate:    %.0f Hz\n", ifrate);

    srcsdr->print_specific_parms();

    // Create source data queue.
    DataBuffer<IQSample> source_buffer;

    // ownership will be transferred to thread therefore the unique_ptr with move is convenient
    // if the pointer is to be shared with the main thread use shared_ptr (and no move) instead
    std::unique_ptr<Source> up_srcsdr(srcsdr);

    // Start reading from device in separate thread.
    //std::thread source_thread(read_source_data, std::move(up_srcsdr), &source_buffer);
    up_srcsdr->start(&source_buffer, &stop_flag);

    if (!up_srcsdr)
    {
    	fprintf(stderr, "ERROR: source: %s\n", up_srcsdr->error().c_str());
    	exit(1);
    }

    // If buffering enabled, start background output thread.
    DataBuffer<IQSample> output_buffer;
    std::thread output_thread;

    if (outputbuf_samples > 0)
    {
        output_thread = std::thread(write_output_data,
                               udp_output.get(),
                               &output_buffer,
                               outputbuf_samples);
    }

    IQSampleVector outsamples;
    bool inbuf_length_warning = false;

    double block_time = get_time();

    // Main loop.
    for (unsigned int block = 0; !stop_flag.load(); block++)
    {

        // Check for overflow of source buffer.
        if (!inbuf_length_warning && source_buffer.queued_samples() > 10 * ifrate)
        {
            fprintf(stderr, "\nWARNING: Input buffer is growing (system too slow)\n");
            inbuf_length_warning = true;
        }

        // Pull next block from source buffer.
        IQSampleVector iqsamples = source_buffer.pull();

        if (iqsamples.empty())
        {
            break;
        }

        block_time = get_time();

        // Possible downsampling and write to UDP

        if (dn.noDecimation())
        {
        	if (outputbuf_samples > 0)
        	{
				// Buffered write.
				output_buffer.push(move(iqsamples));
        	}
			else
			{
				// Direct write.
				udp_output->write(iqsamples);
			}
        }
        else
        {
			dn.process(iqsamples, outsamples);

			// Throw away first block. It is noisy because IF filters
			// are still starting up.
			if (block > 0)
			{
				// Write samples to output.
				if (outputbuf_samples > 0)
				{
					// Buffered write.
					output_buffer.push(move(outsamples));
				}
				else
				{
					// Direct write.
					udp_output->write(outsamples);
				}
			}
        }
    }

    fprintf(stderr, "\n");

    // Join background threads.
    //source_thread.join();
    up_srcsdr->stop();
    
    if (outputbuf_samples > 0)
    {
        output_buffer.push_end();
        output_thread.join();
    }

    // No cleanup needed; everything handled by destructors

    return 0;
}

/* end */
