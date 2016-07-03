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

#ifndef INCLUDE_UDPSINKUNCOMPRESSED_H_
#define INCLUDE_UDPSINKUNCOMPRESSED_H_

#include <atomic>
#include "UDPSink.h"

class UDPSinkUncompressed : public UDPSink
{
public:
	UDPSinkUncompressed(const std::string& address, unsigned int port, unsigned int udpSize);
	virtual ~UDPSinkUncompressed();
	virtual void write(const IQSampleVector& samples_in);
	virtual void setTxDelay(int txDelay) { m_txDelay = txDelay; }

private:
	std::atomic_int m_txDelay; //!< Delay in microseconds (usleep) between each sending of an UDP datagram
};

#endif /* INCLUDE_UDPSINKUNCOMPRESSED_H_ */
