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

typedef struct {
  phr_chunked_decoder_t decoder;
  size_t last_len;
} mrb_phr_t;

static const struct mrb_data_type mrb_phr_type = {
  "$i_phr_t", mrb_free
};

#endif
