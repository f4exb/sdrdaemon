#define SDRDAEMONFEC_API

%include <gnuradio.i>
%include "sdrdaemonfec_swig_doc.i"

%{
#include "sdrdaemonfec/sdrdmnfecsource.h"
%}

%include "sdrdaemonfec/sdrdmnfecsource.h"

GR_SWIG_BLOCK_MAGIC2(sdrdaemonfec, sdrdmnfec_source);
