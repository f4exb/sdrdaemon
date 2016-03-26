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

#include <cstdlib>
#include <cstdio>
#include <climits>
#include <cstring>
#include <cassert>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <unistd.h>
#include <getopt.h>
#include <nanomsg/nn.h>
#include <nanomsg/pair.h>

static void badarg(const char *label);
static bool parse_int(const char *s, int& v, bool allow_unit);

void usage()
{
    fprintf(stderr,
    "Usage: sdrdmnctl [options]\n"
            "  -c config      Configuration commands. Comma separated key=value configuration pairs\n"
            "                 or just key for switches. See sdrdaemon help for valid values for targetted device\n"
            "  -I address     IP address. Configuration commands are sent to this address (default: 127.0.0.1)\n"
            "  -C port        Configuration port (default 9091). The configuration string\n"
            "                 is sent on this port via nanomsg in TCP to control the device\n"
            "  -t timeout     Send timeout in seconds (default 2)\n"
            "\n");
}

void badarg(const char *label)
{
    usage();
    fprintf(stderr, "ERROR: Invalid argument for %s\n", label);
    exit(1);
}

bool parse_int(const char *s, int& v, bool allow_unit=false)
{
    char *endp;
    long t = strtol(s, &endp, 10);
    if (endp == s)
        return false;
    if ( allow_unit && *endp == 'k' &&
         t > INT_MIN / 1000 && t < INT_MAX / 1000 ) {
        t *= 1000;
        endp++;
    }
    if (*endp != '\0' || t < INT_MIN || t > INT_MAX)
        return false;
    v = t;
    return true;
}

int main(int argc, char **argv)
{
    fprintf(stderr,
            "sdrdmnctl - Send SDRdaemon instance a configuration string for its attached device using nanomsg\n");

    const struct option longopts[] = {
        { "config",     2, NULL, 'c' },
        { "daddress",   2, NULL, 'I' },
        { "dport",      1, NULL, 'D' },
        { "timeout",    1, NULL, 't' },
        { NULL,         0, NULL, 0 } };

    int c, longindex, value;
    std::string config_str;
    std::string cmdaddress("127.0.0.1");
    unsigned int cfgport = 9091;
    unsigned int timeout = 2;

    while ((c = getopt_long(argc, argv,
            "t:c:I:C:",
            longopts, &longindex)) >= 0)
    {
        switch (c)
        {
        case 'c':
            config_str.assign(optarg);
            break;
        case 'I':
        	cmdaddress.assign(optarg);
            break;
        case 'C':
            if (!parse_int(optarg, value) || (value < 0)) {
                badarg("-C");
            } else {
           		cfgport = value;
            }
            break;
        case 't':
            if (!parse_int(optarg, value) || (value < 0)) {
                badarg("-C");
            } else {
            	timeout = value;
            }
            break;
        default:
            usage();
            fprintf(stderr, "ERROR: Invalid command line options\n");
            exit(1);

        }
    }

    int sender = nn_socket(AF_SP, NN_PAIR);
    assert(sender != -1);

    int millis = timeout * 1000;
    int rc = nn_setsockopt (sender, NN_SOL_SOCKET, NN_SNDTIMEO,
                       &millis, sizeof (millis));
    assert (rc == 0);

	std::ostringstream os;
	os << "tcp://" << cmdaddress << ":" << cfgport;
    rc = nn_connect(sender, os.str().c_str());
    assert(rc >= 0);

    int config_size = config_str.size();
    rc = nn_send(sender, (void *) config_str.c_str(), config_str.size(), 0);

    if (rc != config_size)
    {
    	std::cerr << "Error sending message" << std::endl;
    }

	return 0;
}
