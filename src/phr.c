#include "picohttpparser.h"
#include <mruby.h>
#include <mruby/data.h>
#include <mruby/variable.h>
#include <mruby/array.h>
#include <mruby/string.h>
#include <mruby/class.h>

#define PHR_MAX_HEADERS 255

typedef struct phr_chunked_decoder phr_chunked_decoder_t;

static const struct mrb_data_type phr_chunked_decoder_type = {
  "$i_phr_chunked_decoder", mrb_free,
};

static mrb_value
phr_chunked_decoder_init(mrb_state *mrb, mrb_value self)
{
  phr_chunked_decoder_t *decoder;

  decoder = (phr_chunked_decoder_t *) DATA_PTR(self);
  if(decoder)
    mrb_free(mrb, decoder);

  mrb_data_init(self, NULL, &phr_chunked_decoder_type);
  decoder = (phr_chunked_decoder_t *) mrb_calloc(mrb, 1,
    sizeof(*decoder));

  mrb_data_init(self, decoder, &phr_chunked_decoder_type);

  return self;
}

static mrb_value
mrb_phr_init(mrb_state *mrb, mrb_value self)
{
  mrb_iv_set(mrb, self,
    mrb_intern_lit(mrb, "last_len"),
    mrb_fixnum_value(0));

  return self;
}

static mrb_value
mrb_phr_method(mrb_state *mrb, mrb_value self)
{
  return mrb_iv_get(mrb, self,
    mrb_intern_lit(mrb, "method"));
}

static mrb_value
mrb_phr_path(mrb_state *mrb, mrb_value self)
{
  return mrb_iv_get(mrb, self,
    mrb_intern_lit(mrb, "path"));
}

static mrb_value
mrb_phr_minor_version(mrb_state *mrb, mrb_value self)
{
  return mrb_iv_get(mrb, self,
    mrb_intern_lit(mrb, "minor_version"));
}

static mrb_value
mrb_phr_status(mrb_state *mrb, mrb_value self)
{
  return mrb_iv_get(mrb, self,
    mrb_intern_lit(mrb, "status"));
}

static mrb_value
mrb_phr_msg(mrb_state *mrb, mrb_value self)
{
  return mrb_iv_get(mrb, self,
    mrb_intern_lit(mrb, "msg"));
}

static mrb_value
mrb_phr_headers(mrb_state *mrb, mrb_value self)
{
  return mrb_iv_get(mrb, self,
    mrb_intern_lit(mrb, "headers"));
}

static inline mrb_value
mrb_phr_headers_to_a(mrb_state *mrb, struct phr_header *headers, size_t num_headers)
{
  mrb_value headers_array = mrb_ary_new_capa(mrb, (mrb_int)num_headers);
  mrb_int i;
  for(i=0;i != num_headers;i++) {
    mrb_value tmp = mrb_ary_new_capa(mrb, 2);
    mrb_ary_set(mrb, tmp, 0,
        mrb_str_new_static(mrb, headers[i].name, headers[i].name_len));
    mrb_ary_set(mrb, tmp, 1,
        mrb_str_new_static(mrb, headers[i].value, headers[i].value_len));
    mrb_ary_set(mrb, headers_array, i, tmp);
  }

  return headers_array;
}

static mrb_value
mrb_phr_parse_request(mrb_state *mrb, mrb_value self)
{
  char *request;
  mrb_int request_len;
  int pret;
  const char *method;
  size_t method_len;
  const char *path;
  size_t path_len;
  int minor_version;
  size_t num_headers = PHR_MAX_HEADERS;
  struct phr_header headers[num_headers];
  mrb_int last_len = mrb_int(mrb, mrb_iv_get(mrb, self,
    mrb_intern_lit(mrb, "last_len")));

  mrb_get_args(mrb, "s", &request, &request_len);

  pret = phr_parse_request(request, request_len,
      &method, &method_len, &path, &path_len, &minor_version, headers,
      &num_headers, last_len);

  if (pret == -1)
    return mrb_symbol_value(mrb_intern_lit(mrb, "parser_error"));

  else
  if (pret == -2) {
    mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "last_len"),
      mrb_fixnum_value(request_len));

    return mrb_symbol_value(mrb_intern_lit(mrb, "incomplete"));
  } else {
    mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "method"),
      mrb_str_new_static(mrb, method, method_len));
    mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "path"),
      mrb_str_new_static(mrb, path, path_len));
    mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "minor_version"),
      mrb_fixnum_value(minor_version));

    mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "headers"),
      mrb_phr_headers_to_a(mrb, headers, num_headers));

    return mrb_fixnum_value(pret);
  }
}

static mrb_value
mrb_phr_parse_response(mrb_state *mrb, mrb_value self)
{
  char *response;
  mrb_int response_len;
  int pret, minor_version, status;
  const char *msg;
  size_t msg_len;
  size_t num_headers = PHR_MAX_HEADERS;
  struct phr_header headers[num_headers];
  mrb_int last_len = mrb_int(mrb, mrb_iv_get(mrb, self,
    mrb_intern_lit(mrb, "last_len")));

  mrb_get_args(mrb, "s", &response, &response_len);

  pret = phr_parse_response(response, response_len,
      &minor_version, &status, &msg, &msg_len, headers,
      &num_headers, last_len);

  if (pret == -1)
    return mrb_symbol_value(mrb_intern_lit(mrb, "parser_error"));

  else
  if (pret == -2) {
    mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "last_len"),
      mrb_fixnum_value(response_len));

    return mrb_symbol_value(mrb_intern_lit(mrb, "incomplete"));
  } else {
    mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "minor_version"),
      mrb_fixnum_value(minor_version));
    mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "status"),
      mrb_fixnum_value(status));
    mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "msg"),
      mrb_str_new_static(mrb, msg, msg_len));

    mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "headers"),
      mrb_phr_headers_to_a(mrb, headers, num_headers));

    return mrb_fixnum_value(pret);
  }
}

static mrb_value
mrb_phr_parse_headers(mrb_state *mrb, mrb_value self)
{
  char *buf;
  mrb_int buf_len;
  int pret;
  size_t num_headers = PHR_MAX_HEADERS;
  struct phr_header headers[num_headers];
  mrb_int last_len = mrb_int(mrb, mrb_iv_get(mrb, self,
    mrb_intern_lit(mrb, "last_len")));

  mrb_get_args(mrb, "s", &buf, &buf_len);
  pret = phr_parse_headers(buf, buf_len, headers, &num_headers, last_len);

  if (pret == -1)
    return mrb_symbol_value(mrb_intern_lit(mrb, "parser_error"));

  else
  if (pret == -2) {
    mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "last_len"),
      mrb_fixnum_value(buf_len));

    return mrb_symbol_value(mrb_intern_lit(mrb, "incomplete"));
  } else {
    mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "headers"),
      mrb_phr_headers_to_a(mrb, headers, num_headers));

    return mrb_fixnum_value(pret);
  }
}

static mrb_value
mrb_phr_decode_chunked(mrb_state *mrb, mrb_value self)
{
  mrb_value buf, block;
  mrb_int rsize;
  int pret;
  phr_chunked_decoder_t *decoder;
  decoder = (phr_chunked_decoder_t *) DATA_PTR(self);

  mrb_get_args(mrb, "S&", &buf, &block);
  rsize = RSTRING_LEN(buf);
  mrb_str_modify(mrb, mrb_str_ptr(buf));
  pret = phr_decode_chunked(decoder, RSTRING_PTR(buf), (size_t *)&rsize);

  if(pret == -1)
    return mrb_symbol_value(mrb_intern_lit(mrb, "parser_error"));

  else
  if (pret == -2) {
    mrb_yield(mrb, block, mrb_str_resize(mrb, buf, rsize));

    return mrb_symbol_value(mrb_intern_lit(mrb, "incomplete"));
  }
  else {
    mrb_yield(mrb, block, mrb_str_resize(mrb, buf, rsize));

    return mrb_fixnum_value(pret);
  }
}

static mrb_value
mrb_consume_trailer(mrb_state *mrb, mrb_value self)
{
  mrb_bool consume;
  mrb_get_args(mrb, "b", &consume);

  phr_chunked_decoder_t *decoder;
  decoder = (phr_chunked_decoder_t *) DATA_PTR(self);

  if (consume == TRUE)
    decoder->consume_trailer = 1;
  else
    decoder->consume_trailer = 0;

  return self;
}

static mrb_value
mrb_phr_reset(mrb_state *mrb, mrb_value self)
{
  (void) mrb_iv_remove(mrb, self,
    mrb_intern_lit(mrb, "last_len"));
  (void) mrb_iv_remove(mrb, self,
    mrb_intern_lit(mrb, "method"));
  (void) mrb_iv_remove(mrb, self,
    mrb_intern_lit(mrb, "path"));
  (void) mrb_iv_remove(mrb, self,
    mrb_intern_lit(mrb, "minor_version"));
  (void) mrb_iv_remove(mrb, self,
    mrb_intern_lit(mrb, "status"));
  (void) mrb_iv_remove(mrb, self,
    mrb_intern_lit(mrb, "msg"));
  (void) mrb_iv_remove(mrb, self,
    mrb_intern_lit(mrb, "headers"));
  (void) mrb_phr_init(mrb, self);

  return self;
}

void
mrb_mruby_phr_gem_init(mrb_state* mrb) {
  struct RClass *phr_class, *phr_chunked_decoder_class;

  phr_class = mrb_define_class(mrb, "Phr", mrb->object_class);

  mrb_define_method(mrb, phr_class, "initialize",       mrb_phr_init,           MRB_ARGS_NONE());
  mrb_define_method(mrb, phr_class, "method",           mrb_phr_method,         MRB_ARGS_NONE());
  mrb_define_method(mrb, phr_class, "path",             mrb_phr_path,           MRB_ARGS_NONE());
  mrb_define_method(mrb, phr_class, "minor_version",    mrb_phr_minor_version,  MRB_ARGS_NONE());
  mrb_define_method(mrb, phr_class, "status",           mrb_phr_status,         MRB_ARGS_NONE());
  mrb_define_method(mrb, phr_class, "msg",              mrb_phr_msg,            MRB_ARGS_NONE());
  mrb_define_method(mrb, phr_class, "headers",          mrb_phr_headers,        MRB_ARGS_NONE());
  mrb_define_method(mrb, phr_class, "parse_request",    mrb_phr_parse_request,  MRB_ARGS_REQ(1));
  mrb_define_method(mrb, phr_class, "parse_response",   mrb_phr_parse_response, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, phr_class, "parse_headers",    mrb_phr_parse_headers,  MRB_ARGS_REQ(1));
  mrb_define_method(mrb, phr_class, "reset",            mrb_phr_reset,          MRB_ARGS_NONE());

  phr_chunked_decoder_class = mrb_define_class_under(mrb, phr_class, "ChunkedDecoder", mrb->object_class);
  MRB_SET_INSTANCE_TT(phr_chunked_decoder_class, MRB_TT_DATA);
  mrb_define_method(mrb, phr_chunked_decoder_class, "initialize",       phr_chunked_decoder_init, MRB_ARGS_NONE());
  mrb_define_method(mrb, phr_chunked_decoder_class, "decode_chunked",   mrb_phr_decode_chunked,   MRB_ARGS_REQ(2));
  mrb_define_method(mrb, phr_chunked_decoder_class, "consume_trailer",  mrb_consume_trailer,      MRB_ARGS_REQ(1));
  mrb_define_method(mrb, phr_chunked_decoder_class, "reset",            phr_chunked_decoder_init, MRB_ARGS_NONE());
}

void
mrb_mruby_phr_gem_final(mrb_state* mrb) {
  /* finalizer */
}
