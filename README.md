SDRdaemon
=========

**SDRdaemon** can be used to send I/Q samples read from a SDR device over the network via UDP.

<h1>Introduction</h1>

**SDRdaemon** is a basic software-defined radio receiver that just sends the I/Q samples over the network via UDP. It was developed on the base of SDRdaemon (also found in this Github repo: https://github.com/f4exb/SDRdaemon) and shares a lot of the code for the interface with the SDR hardware devices.

Hardware supported:

  - **RTL-SDR** based (RTL2832-based) hardware is suppoeted and uses the _librtlsdr_ library to interface with the RTL-SDR hardware.
  - **HackRF** One and variants are supported with _libhackrf_ library.
  - **Airspy** is supported with _libairspy_ library.
  - **BladeRF** is supported with _libbladerf_ library.

SDRdaemon can be used conveniently along with SDRAngel (found in this Github repo: https://github.com/f4exb/sdrangel) as the client application. So in this remote type of configuration you will need both an angel and a daemon :-)

SDRdaemon requires:

 - Linux
 - C++11
 - Boost for compilation
 - RTL-SDR library (http://sdr.osmocom.org/trac/wiki/rtl-sdr)
 - HackRF library (https://github.com/mossmann/hackrf/tree/master/host/libhackrf)
 - Airspy library (https://github.com/airspy/host/tree/master/libairspy)
 - supported RTL-SDR DVB-T receiver or HackRF Rx/Tx
 - A computer or embedded device such as the Raspberry Pi 2. Raspberry Pi B can be used but will run high on CPU so you must provide adequate cooling for a sustained use.

For the latest version, see https://github.com/f4exb/SDRdaemon

Branches:

  - _master_ is the "production" branch with the most stable release
  - _dev_ is the development branch that contains current developments that will be eventually released in the master branch
  - _fix_ contains fixes that cannot wait for the dev branch to go to production


<h1>Prerequisites</h1>

<h2>Base requirements</h2>

  - `sudo apt-get install cmake pkg-config libusb-1.0-0-dev libasound2-dev libboost-all-dev`

<h2>Airspy support</h2>

For now Airspy support must be installed even if no Airspy device is connected.

If you install from source (https://github.com/airspy/host/tree/master/libairspy) in your own installation path you have to specify the include path and library path. For example if you installed it in `/opt/install/libairspy` you have to add `-DLIBAIRSPY_LIBRARIES=/opt/install/libairspy/lib/libairspy.so -DLIBAIRSPY_INCLUDE_DIR=/opt/install/libairspy/include` to the cmake options.

To install the library from a Debian/Ubuntu installation just do: 

  - `sudo apt-get install libairspy-dev`
  
<h2>BladeRF support</h2>

For now BladeRF support must be installed even if no Airspy device is connected.

If you install from source (https://github.com/Nuand/bladeRF) in your own installation path you have to specify the include path and library path. For example if you installed it in `/opt/install/libbladerf` you have to add `-DLIBBLADERF_LIBRARIES=/opt/install/libbladeRF/lib/libbladeRF.so -DLIBBLADERF_INCLUDE_DIR=/opt/install/libbladeRF/include` to the cmake options.

To install the library from a Debian/Ubuntu installation just do: 

  - `sudo apt-get install libbladerf-dev`
  
Note: for the BladeRF to work effectively on FM broadcast frequencies you have to fit it with the XB200 extension board.
  
<h2>HackRF support</h2>

For now HackRF support must be installed even if no HackRF device is connected.

If you install from source (https://github.com/mossmann/hackrf/tree/master/host/libhackrf) in your own installation path you have to specify the include path and library path. For example if you installed it in `/opt/install/libhackrf` you have to add `-DLIBHACKRF_LIBRARIES=/opt/install/libhackrf/lib/libhackrf.so -DLIBHACKRF_INCLUDE_DIR=/opt/install/libhackrf/include` to the cmake options.

To install the library from a Debian/Ubuntu installation just do: 

  - `sudo apt-get install libhackrf-dev`
  
<h2>RTL-SDR support</h2>

The Osmocom RTL-SDR library must be installed before you can build SDRdaemon.
See http://sdr.osmocom.org/trac/wiki/rtl-sdr for more information.
SDRdaemon has been tested successfully with RTL-SDR 0.5.3. Normally your distribution should provide the appropriate librtlsdr package.
If you go with your own installation of librtlsdr you have to specify the include path and library path. For example if you installed it in `-DLIBRTLSDR_LIBRARIES=/opt/install/librtlsdr/lib/librtlsdr.so -DLIBRTLSDR_INCLUDE_DIR=/opt/install/librtlsdr/include` to the cmake options

To install the library from a Debian/Ubuntu installation just do: 

  - `sudo apt-get install librtlsdr-dev`
  
<h1>Installing</h1>

To install SDRdaemon, download and unpack the source code and go to the
top level directory. Then do like this:

 - `mkdir build`
 - `cd build`
 - `cmake ..`

CMake tries to find librtlsdr. If this fails, you need to specify
the location of the library in one the following ways:

 - `cmake .. -DRTLSDR_INCLUDE_DIR=/path/rtlsdr/include -DRTLSDR_LIBRARY_PATH=/path/rtlsdr/lib/librtlsdr.a`
 - `PKG_CONFIG_PATH=/path/to/rtlsdr/lib/pkgconfig cmake ..`

Compile and install

 - `make -j8` (for machines with 8 CPUs)
 - `make install`
 

<h1>Running</h1>

<h2>Examples</h2>

Basic usage:

 - `./SDRdaemon -t rrtlsdr -c freq=94600000` Use RTL-SDR device #0 and start tuned to 94.6 MHz

Specify gain:

 - `./SDRdaemon -t rtlsdr -c freq=94600000,gain=22.9` Use RTL-SDR device #0 and start tuned to 94.6 MHz with a gain of 22.9 dB

<h2>All options</h2>

 - `-t devtype` is mandatory and must be `rtlsdr` for RTL-SDR devices or `hackrf` for HackRF.
 - `-c config` Comma separated list of configuration options as key=value pairs or just key for switches. Depends on device type (see next paragraph).
 - `-d devidx` Device index, 'list' to show device list (default 0)
 - `-r pcmrate` Audio sample rate in Hz (default 48000 Hz)
 - `-M ` Disable stereo decoding
 - `-R filename` Write audio data as raw S16_LE samples. Uuse filename `-` to write to stdout
 - `-W filename` Write audio data to .WAV file
 - `-P [device]` Play audio via ALSA device (default `default`). Use `aplay -L` to get the list of devices for your system
 - `-T filename` Write pulse-per-second timestamps. Use filename '-' to write to stdout
 - `-b seconds` Set audio buffer size in seconds

<h2>Device type specific configuration options</h2>

Note that these options can be used both as the initial configuration as the argument of the `-c` option and as the dynamic configuration sent on the UDP configuration port specified by the `-C` option. 

<h3>RTL-SDR</h3>

  - `freq=<int>` Desired tune frequency in Hz. Accepted range from 10M to 2.2G. (default 100M: `100000000`)
  - `gain=<x>` (default `auto`)
    - `auto` Selects gain automatically
    - `list` Lists available gains and exit
    - `<float>` gain in dB. Possible gains in dB are: `0.0, 0.9, 1.4, 2.7, 3.7, 7.7, 8.7, 12.5, 14.4, 15.7, 16.6, 19.7, 20.7, 22.9, 25.4, 28.0, 29.7, 32.8, 33.8, 36.4, 37.2, 38.6, 40.2, 42.1, 43.4, 43.9, 44.5, 48.0, 49.6`
  - `srate=<int>` Device sample rate. valid values in the [225001, 300000], [900001, 3200000] ranges. (default `1000000`)
  - `decim=<int>` log2 of the decimation factor. Samples collected from the device are downsampled by two to the power of this value. For a value greater than 0 (thus an effective downsampling) the size of the samples is increased to 2x16 bits.
  - `blklen=<int>` Device block length in bytes (default RTL-SDR default i.e. 64k)
  - `agc` Activates device AGC (default off)

<h3>HackRF</h3>

  - `freq=<int>` Desired tune frequency in Hz. Valid range from 1M to 6G. (default 100M: `100000000`)
  - `srate=<int>` Device sample rate (default `5000000`). Valid values from 1M to 20M. In fact rates lower than 10M are not specified in the datasheets of the ADC chip however a rate of `1000000` (1M) still works well with SDRdaemon.
  - `decim=<int>` log2 of the decimation factor. Samples collected from the device are downsampled by two to the power of this value. For a value greater than 0 (thus an effective downsampling) the size of the samples is increased to 2x16 bits.  
  - `lgain=<x>` LNA gain in dB. Valid values are: `0, 8, 16, 24, 32, 40, list`. `list` lists valid values and exits. (default `16`)
  - `vgain=<x>` VGA gain in dB. Valid values are: `0, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 22, 24, 26, 28, 30, 32, 34, 36, 38, 40, 42, 44, 46, 48, 50, 52, 54, 56, 58, 60, 62, list`. `list` lists valid values and exits. (default `22`)
  - `bwfilter=<x>` RF (IF) filter bandwith in MHz. Actual value is taken as the closest to the following values: `1.75, 2.5, 3.5, 5, 5.5, 6, 7,  8, 9, 10, 12, 14, 15, 20, 24, 28, list`. `list` lists valid values and exits. (default `2.5`)
  - `extamp` Turn on the extra amplifier (default off)
  - `antbias` Turn on the antenna bias for remote LNA (default off)
  
<h3>Airspy</h3>

  - `freq=<int>` Desired tune frequency in Hz. Valid range from 1M to 1.8G. (default 100M: `100000000`)
  - `srate=<int>` Device sample rate. `list` lists valid values and exits. (default `10000000`). Valid values depend on the Airspy firmware. Airspy firmware and library must support dynamic sample rate query. 
  - `decim=<int>` log2 of the decimation factor. Samples collected from the device are downsampled by two to the power of this value.
  - `lgain=<x>` LNA gain in dB. Valid values are: `0, 1, 2, 3, 4, 5, 6, 7, 8 ,9 ,10, 11 12, 13, 14, list`. `list` lists valid values and exits. (default `8`)
  - `mgain=<x>` Mixer gain in dB. Valid values are: `0, 1, 2, 3, 4, 5, 6, 7, 8 ,9 ,10, 11 12, 13, 14, 15, list`. `list` lists valid values and exits. (default `8`)
  - `vgain=<x>` VGA gain in dB. Valid values are: `0, 1, 2, 3, 4, 5, 6, 7, 8 ,9 ,10, 11 12, 13, 14, 15, list`. `list` lists valid values and exits. (default `0`)  
  - `antbias` Turn on the antenna bias for remote LNA (default off)
  - `lagc` Turn on the LNA AGC (default off)
  - `magc` Turn on the mixer AGC (default off)
  
<h3>BladeRF</h3>

  - `freq=<int>` Desired tune frequency in Hz. Valid range low boundary depends whether the XB200 extension board is fitted (default `300000000`). 
    - XB200 fitted: 100kHz to 3,8 GHz
    - XB200 not fitted: 300 MHZ to 3.8 GHz.
  - `srate=<int>` Device sample rate in Hz. Valid range is 48kHZ to 40MHz. (default `1000000`).
  - `decim=<int>` log2 of the decimation factor. Samples collected from the device are downsampled by two to the power of this value.
  - `bw=<int>` IF filter bandwidth in Hz. `list` lists valid values and exits. (default `1500000`).
  - `lgain=<x>` LNA gain in dB. Valid values are: `0, 3, 6, list`. `list` lists valid values and exits. (default `3`)
  - `v1gain=<x>` VGA1 gain in dB. Valid values are: `5, 6, 7, 8 ,9 ,10, 11 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, list`. `list` lists valid values and exits. (default `20`)  
  - `v2gain=<x>` VGA2 gain in dB. Valid values are: `0, 3, 6, 9, 12, 15, 18, 21, 24, 27, 30, list`. `list` lists valid values and exits. (default `9`)  

<h1>Data formats</h1>

<h2>I/Q samples and related data</h2>

<h2>Configuration options</h2>

A string of comma separated key=value pairs in the same format as described in the device related options can be sent on the UDP configuration port to dynamically control the device. 

<h1>GNUradio supoort</h1>

A source block is available in the _gr-sdrdaemon_ subdirectory. This subdirectory is a complete OOT module that can be built independently following GNUradio standards. Please refer to the documentation found in this directory for further information.

<h1>License</h1>

**SDRdaemon**, copyright (C) 2015, Edouard Griffiths, F4EXB

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, see http://www.gnu.org/licenses/gpl-2.0.html
