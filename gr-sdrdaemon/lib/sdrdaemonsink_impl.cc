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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "sdrdaemonsink_impl.h"

namespace gr {
  namespace sdrdaemon {

    sdrdaemonsink::sptr
    sdrdaemonsink::make(std::size_t itemsize)
    {
      return gnuradio::get_initial_sptr
        (new sdrdaemonsink_impl(itemsize));
    }

    /*
     * The private constructor
     */
    sdrdaemonsink_impl::sdrdaemonsink_impl(std::size_t itemsize)
      : gr::sync_block("sdrdaemonsink",
              gr::io_signature::make(1, 1, itemsize),
              gr::io_signature::make(0, 0, 0))
    {}

    /*
     * Our virtual destructor.
     */
    sdrdaemonsink_impl::~sdrdaemonsink_impl()
    {
    }

    int
    sdrdaemonsink_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
      //const <+ITYPE+> *in = (const <+ITYPE+> *) input_items[0];

      // Do <+signal processing+>

      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

  } /* namespace sdrdaemon */
} /* namespace gr */

