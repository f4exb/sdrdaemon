///////////////////////////////////////////////////////////////////////////////////
// SDRdaemon - receive I/Q samples over the network via UDP and write to a       //
//             SDR device .                                                      //
//                                                                               //
// Copyright (C) 2017 Edouard Griffiths, F4EXB                                   //
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

#include "DeviceSink.h"

#include "Upsampler.h"

#include <iostream>

bool DeviceSink::configure(std::string& configureStr)
{
    namespace qi = boost::spirit::qi;
    parsekv::key_value_sequence<std::string::iterator> p;
    parsekv::pairs_type m;

    if (!qi::parse(configureStr.begin(), configureStr.end(), p, m))
    {
        fprintf(stderr, "Sink::configure: configuration parsing failed\n");
        return false;
    }
    else
    {
        // configuration for the upsampler

        if (m_upsampler)
        {
            if (!m_upsampler->configure(m))
            {
                fprintf(stderr, "Sink::configure:: ERROR: upsampler configuration: %s\n", m_upsampler->error().c_str());
                return false;
            }
        }

        // configuration for the UDP sink

        if (m.find("fecblk") != m.end())
        {
            int nbFECBlocks = atoi(m["fecblk"].c_str());

            if ((nbFECBlocks >= 1) || (nbFECBlocks < 128))
            {
                m_nbFECBlocks = nbFECBlocks;
            }
        }

        // configuration for the source itself

        return configure(m);
    }
}
