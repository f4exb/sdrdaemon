#define SDRDAEMON_API

%include <gnuradio.i>
%include "sdrdaemon_swig_doc.i"

%{
#include "sdrdaemon/sdrdmnsource.h"
%}

%include "sdrdaemon/sdrdmnsource.h"

GR_SWIG_BLOCK_MAGIC2(sdrdaemon, sdrdmn_source);
