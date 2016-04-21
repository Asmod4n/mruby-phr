#include "mrb_phr.h"

static mrb_value
mrb_phr_init(mrb_state *mrb, mrb_value self)
{
  phr_chunked_decoder_t *chunked_decoder = (phr_chunked_decoder_t *) mrb_calloc(mrb, 1, sizeof(phr_chunked_decoder_t));
  mrb_data_init(self, chunked_decoder, &phr_chunked_decoder_type);
  mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "last_len"), mrb_fixnum_value(0));

  return self;
}

static inline mrb_value
mrb_phr_headers_to_a(mrb_state *mrb, mrb_value buff_obj, struct phr_header *headers, size_t num_headers)
{
  mrb_value headers_array = mrb_ary_new_capa(mrb, num_headers);
  int ai = mrb_gc_arena_save(mrb);
  for (size_t curr_header = 0; curr_header < num_headers; curr_header++) {
    mrb_value header_name = mrb_str_substr(mrb, buff_obj, headers[curr_header].name - RSTRING_PTR(buff_obj), headers[curr_header].name_len);
    mrb_funcall(mrb, header_name, "downcase!", 0);
    mrb_value header_value = mrb_str_substr(mrb, buff_obj, headers[curr_header].value - RSTRING_PTR(buff_obj), headers[curr_header].value_len);
    mrb_value name_value_pair = mrb_assoc_new(mrb, header_name, header_value);

    mrb_ary_push(mrb, headers_array, name_value_pair);
    mrb_gc_arena_restore(mrb, ai);
  }

  return headers_array;
}

static mrb_value
mrb_phr_parse_request(mrb_state *mrb, mrb_value self)
{
  mrb_value request_obj;

  mrb_get_args(mrb, "S", &request_obj);

  const char *method;
  size_t method_len;
  const char *path;
  size_t path_len;
  int minor_version;
  struct phr_header headers[PHR_MAX_HEADERS];
  size_t num_headers = sizeof(headers) / sizeof(headers[0]);
  mrb_int last_len = mrb_fixnum(mrb_iv_get(mrb, self,
    mrb_intern_lit(mrb, "last_len")));

  int pret = phr_parse_request(RSTRING_PTR(request_obj), RSTRING_LEN(request_obj),
      &method, &method_len, &path, &path_len, &minor_version, headers,
      &num_headers, last_len);

  if (pret == -1) {
    return mrb_symbol_value(mrb_intern_lit(mrb, "parser_error"));
  }
  else if (pret == -2) {
    mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "last_len"),
      mrb_fixnum_value(RSTRING_LEN(request_obj)));

    return mrb_symbol_value(mrb_intern_lit(mrb, "incomplete"));
  } else {
    mrb_value method_val = mrb_str_substr(mrb, request_obj, method - RSTRING_PTR(request_obj), method_len);
    mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "@method"), method_val);
    mrb_value path_val = mrb_str_substr(mrb, request_obj, path - RSTRING_PTR(request_obj), path_len);
    mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "@path"), path_val);
    mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "@minor_version"), mrb_fixnum_value(minor_version));
    mrb_value headers_val = mrb_phr_headers_to_a(mrb, request_obj, headers, num_headers);
    mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "@headers"), headers_val);

    return mrb_fixnum_value(pret);
  }
}

static mrb_value
mrb_phr_parse_response(mrb_state *mrb, mrb_value self)
{
  mrb_value response_obj;

  mrb_get_args(mrb, "S", &response_obj);

  int minor_version, status;
  const char *msg;
  size_t msg_len;
  struct phr_header headers[PHR_MAX_HEADERS];
  size_t num_headers = sizeof(headers) / sizeof(headers[0]);
  mrb_int last_len = mrb_fixnum(mrb_iv_get(mrb, self,
    mrb_intern_lit(mrb, "last_len")));

  int pret = phr_parse_response(RSTRING_PTR(response_obj), RSTRING_LEN(response_obj),
      &minor_version, &status, &msg, &msg_len, headers,
      &num_headers, last_len);

  if (pret == -1) {
    return mrb_symbol_value(mrb_intern_lit(mrb, "parser_error"));
  }
  else if (pret == -2) {
    mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "last_len"),
      mrb_fixnum_value(RSTRING_LEN(response_obj)));

    return mrb_symbol_value(mrb_intern_lit(mrb, "incomplete"));
  } else {
    mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "@minor_version"), mrb_fixnum_value(minor_version));
    mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "@status"), mrb_fixnum_value(status));
    mrb_value msg_val = mrb_str_substr(mrb, response_obj, msg - RSTRING_PTR(response_obj), msg_len);
    mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "@msg"), msg_val);
    mrb_value headers_val = mrb_phr_headers_to_a(mrb, response_obj, headers, num_headers);
    mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "@headers"), headers_val);

    return mrb_fixnum_value(pret);
  }
}

static mrb_value
mrb_phr_parse_headers(mrb_state *mrb, mrb_value self)
{
  mrb_value headers_obj;

  mrb_get_args(mrb, "S", &headers_obj);

  struct phr_header headers[PHR_MAX_HEADERS];
  size_t num_headers = sizeof(headers) / sizeof(headers[0]);
  mrb_int last_len = mrb_fixnum(mrb_iv_get(mrb, self,
    mrb_intern_lit(mrb, "last_len")));

  int pret = phr_parse_headers(RSTRING_PTR(headers_obj), RSTRING_LEN(headers_obj),
    headers, &num_headers, last_len);

  if (pret == -1) {
    return mrb_symbol_value(mrb_intern_lit(mrb, "parser_error"));
  }
  else if (pret == -2) {
    mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "last_len"),
      mrb_fixnum_value(RSTRING_LEN(headers_obj)));

    return mrb_symbol_value(mrb_intern_lit(mrb, "incomplete"));
  } else {
    mrb_value headers_val = mrb_phr_headers_to_a(mrb, headers_obj, headers, num_headers);
    mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "@headers"), headers_val);

    return mrb_fixnum_value(pret);
  }
}

static mrb_value
mrb_phr_decode_chunked(mrb_state *mrb, mrb_value self)
{
  mrb_value buf;

  mrb_get_args(mrb, "S", &buf);

  mrb_str_modify(mrb, RSTRING(buf));
  size_t rsize = RSTRING_LEN(buf);
  int pret = phr_decode_chunked((phr_chunked_decoder_t *) DATA_PTR(self), RSTRING_PTR(buf), &rsize);

  if (pret == -1) {
    return mrb_symbol_value(mrb_intern_lit(mrb, "parser_error"));
  }
  else if (pret == -2) {
    mrb_str_resize(mrb, buf, rsize);

    return mrb_symbol_value(mrb_intern_lit(mrb, "incomplete"));
  }
  else {
    mrb_str_resize(mrb, buf, rsize);

    return mrb_fixnum_value(pret);
  }
}

static mrb_value
mrb_consume_trailer(mrb_state *mrb, mrb_value self)
{
  if (((phr_chunked_decoder_t *) DATA_PTR(self))->consume_trailer) {
    return mrb_true_value();
  } else {
    return mrb_false_value();
  }
}

static mrb_value
mrb_set_consume_trailer(mrb_state *mrb, mrb_value self)
{
  mrb_bool consume;

  mrb_get_args(mrb, "b", &consume);

  ((phr_chunked_decoder_t *) DATA_PTR(self))->consume_trailer = consume;

  return self;
}

static mrb_value
mrb_phr_reset(mrb_state *mrb, mrb_value self)
{
  mrb_iv_remove(mrb, self, mrb_intern_lit(mrb, "@method"));
  mrb_iv_remove(mrb, self, mrb_intern_lit(mrb, "@path"));
  mrb_iv_remove(mrb, self, mrb_intern_lit(mrb, "@minor_version"));
  mrb_iv_remove(mrb, self, mrb_intern_lit(mrb, "@status"));
  mrb_iv_remove(mrb, self, mrb_intern_lit(mrb, "@msg"));
  mrb_iv_remove(mrb, self, mrb_intern_lit(mrb, "@headers"));
  mrb_iv_set(mrb, self, mrb_intern_lit(mrb, "last_len"), mrb_fixnum_value(0));
  memset(DATA_PTR(self), 0, sizeof(phr_chunked_decoder_t));

  return self;
}

void
mrb_mruby_phr_gem_init(mrb_state* mrb)
{
  struct RClass *phr_class = mrb_define_class(mrb, "Phr", mrb->object_class);
  MRB_SET_INSTANCE_TT(phr_class, MRB_TT_DATA);
  mrb_define_method(mrb, phr_class, "initialize",       mrb_phr_init,             MRB_ARGS_NONE());
  mrb_define_method(mrb, phr_class, "parse_request",    mrb_phr_parse_request,    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, phr_class, "parse_response",   mrb_phr_parse_response,   MRB_ARGS_REQ(1));
  mrb_define_method(mrb, phr_class, "parse_headers",    mrb_phr_parse_headers,    MRB_ARGS_REQ(1));
  mrb_define_method(mrb, phr_class, "decode_chunked",   mrb_phr_decode_chunked,   MRB_ARGS_REQ(1));
  mrb_define_method(mrb, phr_class, "consume_trailer?", mrb_consume_trailer,      MRB_ARGS_NONE());
  mrb_define_method(mrb, phr_class, "consume_trailer=", mrb_set_consume_trailer,  MRB_ARGS_REQ(1));
  mrb_define_method(mrb, phr_class, "reset",            mrb_phr_reset,            MRB_ARGS_NONE());
}

void
mrb_mruby_phr_gem_final(mrb_state* mrb)
{
}
