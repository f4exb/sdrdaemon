///////////////////////////////////////////////////////////////////////////////////
// SDRdaemon - send I/Q samples read from a SDR device over the network via UDP. //
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

#ifndef INCLUDE_UTIL_H_
#define INCLUDE_UTIL_H_

#include <cmath>

inline bool parse_dbl(const char *s, double& v)
{
    char *endp;

    v = strtod(s, &endp);

    if (endp == s)
    {
        return false;
    }

    if (*endp == 'k')
    {
        v *= 1.0e3;
        endp++;
    }
    else if (*endp == 'M')
    {
        v *= 1.0e6;
        endp++;
    }
    else if (*endp == 'G')
    {
        v *= 1.0e9;
        endp++;
    }

    return (*endp == '\0');
}

inline float db2P(int db)
{
	return pow(10.0, (db / 10.0));
}

inline float db2A(int db)
{
	return pow(10.0, (db / 20.0));
}

#endif /* INCLUDE_UTIL_H_ */
