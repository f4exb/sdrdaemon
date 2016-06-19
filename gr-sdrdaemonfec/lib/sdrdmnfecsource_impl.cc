///////////////////////////////////////////////////////////////////////////////////
// SDRdaemon - send I/Q samples read from a SDR device over the network via UDP  //
//             with FEC protection. GNUradio interface.                          //
//                                                                               //
// This is an adaptation of the GNUradio UDP source                              //
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sdrdmnfecsource_impl.h"
#include <gnuradio/io_signature.h>
#include <gnuradio/math.h>
#include <stdexcept>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <iostream>

namespace gr {
namespace sdrdaemonfec {

const int sdrdmnfec_source_impl::BUF_SIZE_PAYLOADS = 512;

sdrdmnfec_source::sptr sdrdmnfec_source::make(std::size_t itemsize,
        const std::string &ipaddr,
        int port,
        int payload_size,
        bool eof)
{
    return gnuradio::get_initial_sptr(new sdrdmnfec_source_impl(itemsize, ipaddr, port, payload_size, eof));
}

sdrdmnfec_source_impl::sdrdmnfec_source_impl(size_t itemsize,
        const std::string &host,
        int port,
        int payload_size,
        bool eof) :
        sync_block("sdrdmn_source",
                io_signature::make(0, 0, 0),
                io_signature::make(1, 1, itemsize)),
        d_itemsize(itemsize),
        d_payload_size(payload_size),
        d_eof(eof),
        d_connected(false),
        d_sdrdmnbuf(),
        d_residual(0),
        d_sent(0),
        d_offset(0)
{
    // Give us some more room to play.
    d_rxbuf = new char[BUF_SIZE_PAYLOADS * d_payload_size];
    d_residbuf = new char[BUF_SIZE_PAYLOADS * d_payload_size];

    connect(host, port);
}

sdrdmnfec_source_impl::~sdrdmnfec_source_impl()
{
    if (d_connected)
    {
        disconnect();
    }

    delete[] d_rxbuf;
    delete[] d_residbuf;
}

void sdrdmnfec_source_impl::connect(const std::string &host, int port)
{
    if (d_connected)
    {
        disconnect();
    }

    d_host = host;
    d_port = static_cast<unsigned short>(port);

    std::string s_port;
    s_port = (boost::format("%d") % d_port).str();

    if (host.size() > 0)
    {
        boost::asio::ip::udp::resolver resolver(d_io_service);
        boost::asio::ip::udp::resolver::query query(d_host, s_port, boost::asio::ip::resolver_query_base::passive);
        d_endpoint = *resolver.resolve(query);

        d_socket = new boost::asio::ip::udp::socket(d_io_service);
        d_socket->open(d_endpoint.protocol());

        boost::asio::socket_base::linger loption(true, 0);
        d_socket->set_option(loption);

        boost::asio::socket_base::reuse_address roption(true);
        d_socket->set_option(roption);

        d_socket->bind(d_endpoint);

        start_receive();
        d_udp_thread = gr::thread::thread(
                boost::bind(&sdrdmnfec_source_impl::run_io_service, this));
        d_connected = true;
    }
}

void sdrdmnfec_source_impl::disconnect()
{
    gr::thread::scoped_lock lock(d_setlock);

    if (!d_connected)
    {
        return;
    }

    d_io_service.reset();
    d_io_service.stop();
    d_udp_thread.join();

    d_socket->close();
    delete d_socket;

    d_connected = false;
}

// Return port number of d_socket
int sdrdmnfec_source_impl::get_port(void)
{
    //return d_endpoint.port();
    return d_socket->local_endpoint().port();
}

int sdrdmnfec_source_impl::get_center_freq_khz()
{
    int centerFreq = d_sdrdmnbuf.getOutputMeta().m_centerFrequency;
    return centerFreq == 0 ? 100000 : centerFreq;
}

int sdrdmnfec_source_impl::get_sample_rate_hz()
{
    int sampleRate = d_sdrdmnbuf.getOutputMeta().m_sampleRate;
    return sampleRate == 0 ? 48000 : sampleRate;
}

int sdrdmnfec_source_impl::get_sample_bits()
{
    int sampleBits = d_sdrdmnbuf.getOutputMeta().m_sampleBits;
    return sampleBits == 0 ? 8 : sampleBits;
}

int sdrdmnfec_source_impl::get_cur_nb_blocks()
{
    return d_sdrdmnbuf.getCurNbBlocks();
}

int sdrdmnfec_source_impl::get_cur_nb_recovery()
{
    return d_sdrdmnbuf.getCurNbRecovery();
}

float sdrdmnfec_source_impl::get_avg_nb_blocks()
{
    return d_sdrdmnbuf.getAvgNbBlocks();
}

float sdrdmnfec_source_impl::get_avg_nb_recovery()
{
    return d_sdrdmnbuf.getAvgNbRecovery();
}


void sdrdmnfec_source_impl::start_receive()
{
    d_socket->async_receive_from(
            boost::asio::buffer(
                    (void*) d_rxbuf,
                    d_payload_size),
            d_endpoint_rcvd,
            boost::bind(
                    &sdrdmnfec_source_impl::handle_read,
                    this,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred)
    );
}

void sdrdmnfec_source_impl::handle_read(const boost::system::error_code& error, std::size_t bytes_transferred)
{
    if (!error)
    {
        boost::lock_guard<gr::thread::mutex> lock(d_udp_mutex);

        if (d_eof && (bytes_transferred == 1) && (d_rxbuf[0] == 0x00))
        {
            // If we are using EOF notification, test for it and don't
            // add anything to the output.
            d_residual = WORK_DONE;
            d_cond_wait.notify_one();
            return;
        }
        else
        {
            // Make sure we never go beyond the boundary of the
            // residual buffer.  This will just drop the last bit of
            // data in the buffer if we've run out of room.
            if ((int) (d_residual + bytes_transferred) >= (BUF_SIZE_PAYLOADS * d_payload_size))
            {
                //GR_LOG_WARN(d_logger, "Too much data; dropping packet.");
            }
            else
            {
                // otherwise, copy received data into local buffer for
                // copying later.
                std::size_t dataRead;
                d_sdrdmnbuf.writeAndRead((uint8_t *) d_rxbuf, bytes_transferred, (uint8_t *) d_residbuf + d_residual, dataRead);
                d_residual += dataRead;

                //memcpy(d_residbuf + d_residual, d_rxbuf, bytes_transferred);
                //d_residual += bytes_transferred;
            }
        }

        d_cond_wait.notify_one();
    }

    start_receive();
}

int sdrdmnfec_source_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
{
    gr::thread::scoped_lock l(d_setlock);

    char *out = (char*) output_items[0];

    // Use async receive_from to get data from UDP buffer and wait
    // on a conditional signal before proceeding. We use this
    // because the conditional wait is interruptable while a
    // synchronous receive_from is not.
    boost::unique_lock<boost::mutex> lock(d_udp_mutex);

    //use timed_wait to avoid permanent blocking in the work function
    d_cond_wait.timed_wait(lock, boost::posix_time::milliseconds(10));

    if (d_residual < 0)
    {
        return d_residual;
    }

    int bytes_left_in_buffer = (int) (d_residual - d_sent);
    int bytes_to_send = std::min<int>(d_itemsize * noutput_items, bytes_left_in_buffer);

    // Copy the received data in the residual buffer to the output stream
    memcpy(out, d_residbuf + d_sent, bytes_to_send);
    int nitems = bytes_to_send / d_itemsize;

    // Keep track of where we are if we don't have enough output
    // space to send all the data in the residbuf.
    if (bytes_to_send == bytes_left_in_buffer)
    {
        d_residual = 0;
        d_sent = 0;
    }
    else
    {
        d_sent += bytes_to_send;
    }

    return nitems;
}

} /* namespace blocks */
} /* namespace gr */
