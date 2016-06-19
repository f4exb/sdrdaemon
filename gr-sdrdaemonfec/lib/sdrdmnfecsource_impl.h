///////////////////////////////////////////////////////////////////////////////////
// SDRdaemon - send I/Q samples read from a SDR device over the network via UDP  //
//             with FEC protection. GNUradio interface.                          //
//                                                                               //
// This is an adaptation of the GNUradio UDP source                              //
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

#ifndef INCLUDED_SDRDAEMONFEC_SOURCE_IMPL_H
#define INCLUDED_SDRDAEMONFEC_SOURCE_IMPL_H

#include <boost/asio.hpp>
#include <boost/format.hpp>
#include <gnuradio/thread/thread.h>

#include "../include/sdrdaemonfec/sdrdmnfecsource.h"
#include "SDRdaemonFECBuffer.h"

namespace gr {
namespace sdrdaemonfec {

class sdrdmnfec_source_impl: public sdrdmnfec_source
{
private:
    std::size_t d_itemsize;
    int d_payload_size; // maximum transmission unit (packet length)
    bool d_eof;          // look for an EOF signal
    bool d_connected;    // are we connected?
    char *d_rxbuf;        // get UDP buffer items
    char *d_residbuf;     // hold buffer between calls
    SDRdaemonFECBuffer d_sdrdmnbuf;
    ssize_t d_residual; // hold information about number of bytes stored in residbuf
    ssize_t d_sent;         // track how much of d_residbuf we've outputted
    std::size_t d_offset;       // point to residbuf location offset

    static const int BUF_SIZE_PAYLOADS; //!< The d_residbuf size in multiples of d_payload_size

    std::string d_host;
    unsigned short d_port;

    boost::asio::ip::udp::socket *d_socket;
    boost::asio::ip::udp::endpoint d_endpoint;
    boost::asio::ip::udp::endpoint d_endpoint_rcvd;
    boost::asio::io_service d_io_service;

    gr::thread::condition_variable d_cond_wait;
    gr::thread::mutex d_udp_mutex;
    gr::thread::thread d_udp_thread;

    void start_receive();
    void handle_read(const boost::system::error_code& error, std::size_t bytes_transferred);
    void run_io_service() { d_io_service.run(); }

public:
    sdrdmnfec_source_impl(std::size_t itemsize, const std::string &host, int port,  int payload_size, bool eof);
    ~sdrdmnfec_source_impl();

    void connect(const std::string &host, int port);
    void disconnect();

    int payload_size() { return d_payload_size; }
    int get_port();
    int get_center_freq_khz();
    int get_sample_rate_hz();
    int get_sample_bits();
    int get_cur_nb_blocks();
    int get_cur_nb_recovery();
    float get_avg_nb_blocks();
    float get_avg_nb_recovery();

    int work(int noutput_items, gr_vector_const_void_star &input_items, gr_vector_void_star &output_items);
};

} /* namespace sdrdaemonfec */
} /* namespace gr */

#endif /* INCLUDED_SDRDAEMONFEC_SOURCE_IMPL_H */

