### Dependencies

GNU Radio v3.7.

packets from your packet manager:
```
sudo apt-get install cmake libboost-all-dev liblog4cpp5-dev swig
```


### Installation

Assuming [CM256 library](https://github.com/f4exb/cm256) installed in `/opt/install/cm256`

```
mkdir build
cd build
cmake -DCM256_INCLUDE_DIR=/opt/install/cm256/include/cm256 -DCM256_LIBRARIES=/opt/install/cm256/lib/libcm256.so ..
make
sudo make install
sudo ldconfig
```
With a custom GNU Radio installation in say `/opt/install/gnuradio-3.7.8` you need to use this cmake command: 
`cmake -DGNURADIO_RUNTIME_LIBRARIES="/opt/install/gnuradio-3.7.8/lib/libgnuradio-runtime.so;/opt/install/gnuradio-3.7.8/lib/libgnuradio-pmt.so" -DGNURADIO_RUNTIME_INCLUDE_DIRS=/opt/install/gnuradio-3.7.8/include -DCM256_INCLUDE_DIR=/opt/install/cm256/include/cm256 -DCM256_LIBRARIES=/opt/install/cm256/lib/libcm256.so -DCMAKE_INSTALL_PREFIX:PATH=/opt/install/gnuradio-3.7.8 -Wno-dev ..`

Then you normally don't need sudo to install:

make install

### Usage

open apps/sdrdaemonfec_rx.grc example flow graph in GNU Radio Companion.


### Demos


### History

