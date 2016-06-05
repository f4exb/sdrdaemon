///////////////////////////////////////////////////////////////////////////////////
// SDRdaemon - send I/Q samples read from a SDR device over the network via UDP  //
//             sith FEC protection GNUradio interface.                           //
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

#ifndef INCLUDED_SDRDAEMONFEC_SOURCE_H
#define INCLUDED_SDRDAEMONFEC_SOURCE_H

#include <gnuradio/sync_block.h>
#include "../sdrdaemonfec/api.h"

namespace gr {
namespace sdrdaemonfec {
/*!
* \brief Read SDRdaemon stream from an UDP socket.
*/
class SDRDAEMONFEC_API sdrdmnfec_source : virtual public gr::sync_block
{
public:
	typedef boost::shared_ptr<sdrdmnfec_source> sptr;
	/*!
	* \brief SDRDaemon Source Constructor
	*
	* \param itemsize The size (in bytes) of the item datatype
	* \param host The name or IP address of the transmitting host; can be
	* NULL, None, or "0.0.0.0" to allow reading from any
	* interface on the host
	* \param port The port number on which to receive data; use 0 to
	* have the system assign an unused port number
	* \param payload_size UDP payload size by default set to 1472 =
	* (1500 MTU - (8 byte UDP header) - (20 byte IP header))
	* \param eof Interpret zero-length packet as EOF (default: true)
	*/
	static sptr make(std::size_t itemsize, const std::string &host, int port, int payload_size=1472, bool eof=true);

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

    /*! \brief return compression ratio (1.0 uncompressed, <1.0 compressed, >1.0 expanded) */
    virtual float get_compression_ratio() = 0;
};

} // namespace sdrdaemonfec
} // namespace gr

#endif /* INCLUDED_SDRDAEMONFEC_SOURCE_H */
