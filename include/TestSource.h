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

#ifndef SDRDAEMON_TESTSOURCE_H
#define SDRDAEMON_TESTSOURCE_H

#include <cstdint>
#include <string>
#include <vector>
#include <thread>
#include <zmq.hpp>

#include "Source.h"

class TestSource : public Source
{
public:

    static const int default_block_length = 65536;

    /** Open test device. */
    TestSource(int dev_index);

    /** Close test device. */
    virtual ~TestSource();

    /** Return sample size in bits */
    virtual std::uint32_t get_sample_bits() { return 8; }

    /** Return current sample frequency in Hz. */
    virtual std::uint32_t get_sample_rate();

    /** Return device current center frequency in Hz. */
    virtual std::uint32_t get_frequency();

    /** Print current parameters specific to device type */
    virtual void print_specific_parms();

    virtual bool start(DataBuffer<IQSample>* samples, std::atomic_bool *stop_flag);
    virtual bool stop();

    /** Return true if the device is OK, return false if there is an error. */
    virtual operator bool() const
    {
        return m_error.empty();
    }

    /** Return a list of supported devices. */
    static void get_device_names(std::vector<std::string>& devices);

private:

    /** Configure RTL-SDR tuner from a list of key=values */
    virtual bool configure(parsekv::pairs_type& m);

    /**
     * Configure RTL-SDR tuner and prepare for streaming.
     *
     * changeFlags  :: horrible hack to notify which fields have changed
     * sample_rate  :: desired sample rate in Hz.
     * frequency    :: desired center frequency in Hz.
     * tuner_gain   :: desired tuner gain in 0.1 dB, or INT_MIN for auto-gain.
     * block_length :: preferred number of samples per block.
     *
     * Return true for success, false if an error occurred.
     */
    bool configure(std::uint32_t changeFlags,
    		       std::uint32_t sample_rate,
                   std::uint32_t frequency,
				   float deltaPhase,
                   float tuner_gain,
                   int block_length=default_block_length);

    /**
     * Fetch a bunch of samples from the device.
     *
     * This function must be called regularly to maintain streaming.
     * Return true for success, false if an error occurred.
     */
    static bool get_samples(IQSampleVector *samples);

    static void run();
    static int read_samples(uint8_t *data, int wantSize, int& getSize, float& phasor, int sampleRate, float deltaPhase, float amplitude);

    float getDeltaPhase(int32_t deltaFrequency) const { return 2.0 * M_PI * ((float) deltaFrequency / m_srate); }

    int               m_dev;
    int               m_block_length;
    std::thread       *m_thread;
    static TestSource *m_this;
    float             m_phase;
    float             m_deltaPhase;
    uint64_t          m_freq;
    uint32_t          m_srate;
    float             m_amplitude;
};

#endif // SDRDAEMON_TESTSOURCE_H
