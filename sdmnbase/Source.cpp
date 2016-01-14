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

#include "Source.h"
#include "Downsampler.h"

bool Source::configure(std::string& configureStr)
{
    namespace qi = boost::spirit::qi;
    parsekv::key_value_sequence<std::string::iterator> p;
    parsekv::pairs_type m;

    if (!qi::parse(configureStr.begin(), configureStr.end(), p, m))
    {
        fprintf(stderr, "Configuration parsing failed\n");
        return false;
    }
    else
    {
        if (m_downsampler)
        {
            if (!m_downsampler->configure(m))
            {
                fprintf(stderr, "ERROR: downsampler configuration: %s\n", m_downsampler->error().c_str());
                return false;
            }
        }

        return configure(m);
    }
}
