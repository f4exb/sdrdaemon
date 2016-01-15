#!/usr/bin/env python
import sys
import zmq
from optparse import OptionParser

# ======================================================================
class Error(Exception):
    """Base class for exceptions in this module."""
    pass

# ======================================================================
class InputError(Error):
    """Exception raised for errors in the input.

    Attributes:
        msg  -- explanation of the error
    """

    def __init__(self, msg):
        self.msg = msg

# -------------------------------------------------------------------
def getInputOptions():
# -------------------------------------------------------------------
    parser = OptionParser(usage="usage: %%prog options\n\n%s")
    parser.add_option("-I", "--ip-address", dest="address", help="IP address of the ZMQ server", metavar="ADDRESS", type="string")
    parser.add_option("-P", "--port", dest="port", help="TCP port of the ZMQ server", metavar="PORT", type="int")
    parser.add_option("-m", "--message", dest="message", help="Message to send to the ZMQ server", metavar="STRING", type="string")
    parser.add_option("-t", "--timeout", dest="timeout", help="Send timeout in seconds (default 2)", metavar="SECONDS", type="int")

    (options, args) = parser.parse_args()

    if options.address is None:
        options.address = '127.0.0.1'

    if options.port is None or options.port < 1024:
        options.port = 9091

    if options.message is None:
        options.message = "freq=100000000"

    if options.timeout is None:
        options.timeout = 2;

    return options

# -------------------------------------------------------------------
def main():
# -------------------------------------------------------------------
    try:
    	options = getInputOptions()

        c = zmq.Context()
        s = c.socket(zmq.PAIR)
        s.setsockopt(zmq.LINGER, 0)

        connectStr = 'tcp://%s:%d' % (options.address, options.port)

        s.connect(connectStr)

        mt = s.send(options.message, copy=False, track=True)

        mt.wait(options.timeout)

        if mt.done:
            print >> sys.stderr, "Message sent to", connectStr

        s.disconnect(connectStr)

    except KeyboardInterrupt:
        print >> sys.stderr, "Interrupted"
        pass
    except zmq.error.NotDone:
        print >> sys.stderr, "Message not sent to", connectStr
        pass
    except InputError as e:
        print >> sys.stderr, e.msg
    finally:
        s.close()
        c.destroy()
        print >> sys.stderr, "Good Bye!"


# -------------------------------------------------------------------
if __name__ == "__main__":
    main()
