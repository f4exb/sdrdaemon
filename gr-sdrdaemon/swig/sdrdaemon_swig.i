/* -*- c++ -*- */

#define SDRDAEMON_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "sdrdaemon_swig_doc.i"

%{
#include "sdrdaemon/sdrdaemonsource.h"
#include "sdrdaemon/sdrdaemonsink.h"
%}


%include "sdrdaemon/sdrdaemonsource.h"
GR_SWIG_BLOCK_MAGIC2(sdrdaemon, sdrdaemonsource);
%include "sdrdaemon/sdrdaemonsink.h"
GR_SWIG_BLOCK_MAGIC2(sdrdaemon, sdrdaemonsink);
