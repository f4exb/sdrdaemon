/* -*- c++ -*- */
/*
 * Copyright 2017 <+YOU OR YOUR COMPANY+>.
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */


#ifndef INCLUDED_SDRDAEMON_SDRDAEMONSOURCE_H
#define INCLUDED_SDRDAEMON_SDRDAEMONSOURCE_H

#include <sdrdaemon/api.h>
#include <gnuradio/sync_block.h>

namespace gr {
  namespace sdrdaemon {

    /*!
     * \brief <+description of block+>
     * \ingroup sdrdaemon
     *
     */
    class SDRDAEMON_API sdrdaemonsource : virtual public gr::sync_block
    {
     public:
      typedef boost::shared_ptr<sdrdaemonsource> sptr;

      /*!
       * \brief Return a shared_ptr to a new instance of sdrdaemon::sdrdaemonsource.
       *
       * To avoid accidental use of raw pointers, sdrdaemon::sdrdaemonsource's
       * constructor is in a private implementation
       * class. sdrdaemon::sdrdaemonsource::make is the public interface for
       * creating new instances.
       *
       * \param itemsize The size (in bytes) of the item datatype
       * \param host The name or IP address of the transmitting host; can be
       * NULL, None, or "0.0.0.0" to allow reading from any
       * interface on the host
       * \param port The port number on which to receive data; use 0 to
       * have the system assign an unused port number
       * \param payload_size UDP payload size by default set to 512
       */
      static sptr make(std::size_t itemsize, const std::string &host, int port, int payload_size = 512);

      /*! \brief Change the connection to a new destination
      *
      * \param host The name or IP address of the receiving host; use
      * NULL or None to break the connection without closing
      * \param port Destination port to connect to on receiving host
      *
      * Calls disconnect() to terminate any current connection first.
      */
      virtual void connect(const std::string &host, int port) = 0;

      /*! \brief Cut the connection if we have one set up.
      */
      virtual void disconnect() = 0;

      /*! \brief return the PAYLOAD_SIZE of the socket */
      virtual int payload_size() = 0;

      /*! \brief return the port number of the socket */
      virtual int get_port() = 0;

      /*! \brief return the center frequency in kHz */
      virtual int get_center_freq_khz() = 0;

      /*! \brief return the sample rate in Hz */
      virtual int get_sample_rate_hz() = 0;

      /*! \brief return sample size in bits */
      virtual int get_sample_bits() = 0;

      /*! \brief return the instantaneous number of blocks received */
      virtual int get_cur_nb_blocks() = 0;

      /*! \brief return the instantaneous number of recovery blocks used */
      virtual int get_cur_nb_recovery() = 0;

      /*! \brief return the average number of blocks received */
      virtual float get_avg_nb_blocks() = 0;

      /*! \brief return the average number of recovery blocks used */
      virtual float get_avg_nb_recovery() = 0;
    };

  } // namespace sdrdaemon
} // namespace gr

#endif /* INCLUDED_SDRDAEMON_SDRDAEMONSOURCE_H */

