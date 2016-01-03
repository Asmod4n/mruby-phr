#ifndef MRB_PHR_H
#define MRB_PHR_H

#include "picohttpparser.h"
#include <ctype.h>
#include <string.h>
#include <mruby.h>
#include <mruby/data.h>
#include <mruby/variable.h>
#include <mruby/array.h>
#include <mruby/string.h>
#include <mruby/class.h>

#define PHR_MAX_HEADERS 255U

typedef struct phr_chunked_decoder phr_chunked_decoder_t;

static const struct mrb_data_type phr_chunked_decoder_type = {
  "$i_phr_chunked_decoder", mrb_free
};

#endif
