///////////////////////////////////////////////////////////////////////////////////
// SDRdaemon - receive I/Q samples over the network via UDP and write to a       //
//             SDR device .                                                      //
//                                                                               //
// Copyright (C) 2017 Edouard Griffiths, F4EXB                                   //
//                                                                               //
// This version implements Forward Erasure Correction (FEC) to be able to        //
// recover lost blocks at the receiving end. It uses Cauchy Reed-Solomon coding  //
// for the redundant blocks and is based on the modified cm256 library (see:     //
// https://github.com/f4exb/cm256) with g++ cmake and Neon for armv7l devices    //
// support.                                                                      //
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
#include "DataBuffer.h"
#include "Upsampler.h"
#include "UDPSourceFEC.h"

#ifdef HAS_HACKRF
    #include "HackRFSink.h"
#endif
#include "include/SDRDaemon.h"

//#include <type_traits>

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
 * Get data from input UDP stream and write to input buffer.
 *
 * This code runs in a separate thread.
 */
void read_input_data(UDPSource *input,
        DataBuffer<IQSample> *buf,
        std::size_t buf_maxfill)
{
    IQSampleVector samples;

    while (!stop_flag.load())
    {
        // Get samples from UDP
        input->read(samples);

        if (!(*input))
        {
            fprintf(stderr, "ERROR: Input: %s\n", input->error().c_str());
        }

        if (buf->queued_samples() < buf_maxfill)
        {
            buf->push(move(samples));
        }
        else
        {
            fprintf(stderr, "Device queing up. Dropping samples");
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
    ssize_t r = write(STDERR_FILENO, s, strlen(s));

    if (r != (ssize_t) strlen(s)) {
        msg += " write incomplete";
    }
}


void usage()
{
    fprintf(stderr,
    "Usage: sdrdaemontx [options]\n"
            "  -t devtype     Device type:\n"
#ifdef HAS_HACKRF
            "                   - hackrf:  HackRF One or Jawbreaker\n"
#endif
            "  -c config      Startup configuration. Comma separated key=value configuration pairs\n"
            "                 or just key for switches. See below for valid values\n"
            "  -d devidx      Device index, 'list' to show device list (default 0)\n"
            "  -b             Buffered UDP reads\n"
            "  -I address     IP address. Samples are sent to this address (default: 127.0.0.1)\n"
            "  -D port        Data port. Samples are sent on this UDP port (default 9090)\n"
            "  -C port        Configuration port (default 9091). The configuration string as described below\n"
            "                 is sent on this port via nanomsg in TCP to control the device\n"
            "\n"
            "Configuration options for the interpolator:\n"
            "  interp=<int>   log2 of interpolation factor (default 0: no interpolation)\n"
            "\n"
#ifdef HAS_HACKRF
            "Configuration options for HackRF devices\n"
            "  freq=<int>     Frequency of radio station in Hz (default 100000000)\n"
            "                 valid values: 1M to 6G\n"
            "  srate=<int>    IF sample rate in Hz (default 5000000)\n"
            "                 (valid ranges: [2500000,20000000]))\n"
            "  ppmp=<float>   Set LO correction in positive PPM. Takes precedence over ppmn parameter (default 0)\n"
            "  ppmn=<float>   Set LO correction in negative PPM (default 0)\n"
            "  vgain=<int>    VGA gain in dB. 'list' to just get a list of valid values: (default 22)\n"
            "  bwfilter=<int> Filter bandwidth in MHz. 'list' to just get a list of valid values: (default 2.5)\n"
            "  extamp         Enable extra RF amplifier (default disabled)\n"
            "  antbias        Enable antemma bias (default disabled)\n"
            "\n"
#endif
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


static bool get_device(std::vector<std::string> &devnames, std::string& devtype, Sink **sinksdr, int devidx)
{
    bool deviceDefined = false;
#ifdef HAS_HACKRF
    if (strcasecmp(devtype.c_str(), "hackrf") == 0)
    {
        HackRFSink::get_device_names(devnames);
        deviceDefined = true;
    }
#endif

    if (!deviceDefined)
    {
        fprintf(stderr, "ERROR: wrong device type (-t option) must be one of the following:\n");
#ifdef HAS_HACKRF
        fprintf(stderr, "       hackrf\n");
#endif
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

#ifdef HAS_HACKRF
    if (strcasecmp(devtype.c_str(), "hackrf") == 0)
    {
        // Open HackRF device.
        *sinksdr = new HackRFSink(devidx);
    }
#endif

    return true;
}

int main(int argc, char **argv)
{
    int     devidx  = 0;
    std::string  filename;
    std::string  alsadev("default");
    std::string config_str;
    std::string devtype_str;
    std::vector<std::string> devnames;
    std::string dataaddress("127.0.0.1");
    unsigned int dataport = 9090;
    unsigned int cfgport = 9091;
    Sink  *sinksdr = 0;
    bool buffered_reads = false;

    fprintf(stderr,
            "SDRDaemonTx - Collect samples from network via UDP and send it to SDR device\n");

    const struct option longopts[] = {
        { "devtype",    2, NULL, 't' },
        { "config",     2, NULL, 'c' },
        { "dev",        1, NULL, 'd' },
        { "buffered",   0, NULL, 'b' },
        { "daddress",   2, NULL, 'I' },
        { "dport",      1, NULL, 'D' },
        { "cport",      1, NULL, 'C' },
        { NULL,         0, NULL, 0 } };

    int c, longindex, value;
    while ((c = getopt_long(argc, argv,
            "t:c:d:bI:D:C:",
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
                buffered_reads = true;
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

    // Prepare reader.
    UDPSource *udp_input_instance;
    udp_input_instance = new UDPSourceFEC(dataaddress, dataport);
    std::unique_ptr<UDPSource> udp_input(udp_input_instance);

    if (!(*udp_input))
    {
        fprintf(stderr, "ERROR: UDP Input: %s\n", udp_input->error().c_str());
        exit(1);
    }

    if (!get_device(devnames, devtype_str, &sinksdr, devidx))
    {
        exit(1);
    }

    if (!(*sinksdr))
    {
        fprintf(stderr, "ERROR sink: %s\n", sinksdr->error().c_str());
        delete sinksdr;
        exit(1);
    }

    //fprintf(stderr, (std::is_trivially_copyable<IQSample>::value ? "IQSample is trivially copiable\n" : "IQSample is NOT trivially copiable\n"));

    // Configure device and start streaming.

    sinksdr->setConfigurationPort(cfgport);

    // Prepare upsampler.
    Upsampler up;
    sinksdr->associateUpsampler(&up);

    if (!sinksdr->configure(config_str))
    {
        fprintf(stderr, "ERROR: sink configuration: %s\n", sinksdr->error().c_str());
        delete sinksdr;
        exit(1);
    }

    /*
    if (!dn.configure(m))
    {
        fprintf(stderr, "ERROR: downsampler configuration: %s\n", dn.error().c_str());
        delete srcsdr;
        exit(1);
    }*/

    double freq = sinksdr->get_transmit_frequency();
    fprintf(stderr, "tuned for:         %.6f MHz\n", freq * 1.0e-6);

    double tuner_freq = sinksdr->get_frequency();
    fprintf(stderr, "device tuned for:  %.6f MHz\n", tuner_freq * 1.0e-6);

    double ifrate = sinksdr->get_sample_rate();
    fprintf(stderr, "IF sample rate:    %.0f Hz\n", ifrate);

    sinksdr->print_specific_parms();

    // Create source data queue.
    DataBuffer<IQSample> sink_buffer;

    // ownership will be transferred to thread therefore the unique_ptr with move is convenient
    // if the pointer is to be shared with the main thread use shared_ptr (and no move) instead
    std::unique_ptr<Sink> sinksdr_uptr(sinksdr);

    // Start writing to device in separate thread.
    //std::thread source_thread(read_source_data, std::move(up_srcsdr), &source_buffer);
    sinksdr_uptr->start(&sink_buffer, &stop_flag);

    if (!sinksdr_uptr)
    {
        fprintf(stderr, "ERROR: sink: %s\n", sinksdr_uptr->error().c_str());
        exit(1);
    }

    // If buffering enabled, start background input thread.
    DataBuffer<IQSample> input_buffer;
    std::thread input_thread;

    if (buffered_reads)
    {
        input_thread = std::thread(read_input_data,
                               udp_input.get(),
                               &input_buffer,
                               20 * ifrate);
    }

    IQSampleVector insamples, outsamples;
    bool sink_buf_overflow_warning = false;
    bool sink_buf_underflow_warning = false;

    // Main loop.
    for (unsigned int block = 0; !stop_flag.load(); block++)
    {
        // Check for overflow of sink buffer.
        if (!sink_buf_overflow_warning && sink_buffer.queued_samples() > 10 * ifrate)
        {
            fprintf(stderr, "\nWARNING: Sink buffer is growing (system too fast)\n");
            sink_buf_overflow_warning = true;
        }

        if (sink_buf_overflow_warning && sink_buffer.queued_samples() < 6 * ifrate)
        {
            sink_buf_overflow_warning = false;
        }

        // Check for underflow of sink buffer.
        if (!sink_buf_underflow_warning && sink_buffer.queued_samples() < 2 * ifrate)
        {
            fprintf(stderr, "\nWARNING: Sink buffer is depleting (system too slow)\n");
            sink_buf_underflow_warning = true;
        }

        if (sink_buf_underflow_warning && sink_buffer.queued_samples() > 6 * ifrate)
        {
            sink_buf_underflow_warning = false;
        }

        if (buffered_reads)
        {
            input_buffer.pull(insamples);
        }
        else
        {
            udp_input->read(insamples);
        }

        if (insamples.size() > 0)
        {
            if (up.getLog2Interpolation() == 0)
            {
                sink_buffer.push(move(insamples));
            }
            else
            {
                unsigned int sampleSize = sinksdr->get_sample_bits();
                up.process(sampleSize, insamples, outsamples);
                sink_buffer.push(move(outsamples));
            }
        }
    }

    fprintf(stderr, "\n");

    // Join background threads.
    //source_thread.join();
    sinksdr_uptr->stop();

    if (buffered_reads)
    {
        input_buffer.push_end();
        input_thread.join();
    }

    // No cleanup needed; everything handled by destructors

    return 0;
}

/* end */
