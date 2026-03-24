#include "mrb_phr.h"

#if (__GNUC__ >= 3) || (__INTEL_COMPILER >= 800) || defined(__clang__)
# define likely(x) __builtin_expect(!!(x), 1)
# define unlikely(x) __builtin_expect(!!(x), 0)
#else
# define likely(x) (x)
# define unlikely(x) (x)
#endif

static mrb_value
mrb_phr_init(mrb_state *mrb, mrb_value self)
{
  mrb_phr_t *mrb_phr = mrb_realloc(mrb, DATA_PTR(self), sizeof(*mrb_phr));
  memset(mrb_phr, 0, sizeof(*mrb_phr));
  mrb_data_init(self, mrb_phr, &mrb_phr_type);

  return self;
}

static mrb_value
mrb_phr_headers_to_a(mrb_state *mrb, mrb_value buff_obj, struct phr_header *headers, size_t num_headers)
{
  mrb_value headers_array = mrb_ary_new_capa(mrb, num_headers);
  mrb_gc_protect(mrb, headers_array);
  int ai = mrb_gc_arena_save(mrb);
  for (size_t curr_header = 0; curr_header < num_headers; curr_header++) {
    mrb_value header_name = mrb_nil_value();
    if (likely(headers[curr_header].name)) {
      header_name = mrb_str_substr(mrb, buff_obj, headers[curr_header].name - RSTRING_PTR(buff_obj), headers[curr_header].name_len);
      mrb_gc_protect(mrb, header_name);
    }
    mrb_value header_value = mrb_str_substr(mrb, buff_obj, headers[curr_header].value - RSTRING_PTR(buff_obj), headers[curr_header].value_len);
    mrb_gc_protect(mrb, header_value);

    mrb_ary_push(mrb, headers_array, mrb_assoc_new(mrb, header_name, header_value));
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
  mrb_phr_t *mrb_phr = DATA_PTR(self);


  int pret = phr_parse_request(RSTRING_PTR(request_obj), RSTRING_LEN(request_obj),
      &method, &method_len, &path, &path_len, &minor_version, headers,
      &num_headers, mrb_phr->last_len);

  switch(pret) {
    case -1: {
      return mrb_symbol_value(MRB_SYM(parser_error));
    } break;
    case -2: {
      mrb_phr->last_len = RSTRING_LEN(request_obj);
      return mrb_symbol_value(MRB_SYM(incomplete));
    } break;
    default: {
      mrb_iv_set(mrb, self, MRB_IVSYM(method), mrb_str_substr(mrb, request_obj, method - RSTRING_PTR(request_obj), method_len));
      mrb_iv_set(mrb, self, MRB_IVSYM(path), mrb_str_substr(mrb, request_obj, path - RSTRING_PTR(request_obj), path_len));
      mrb_iv_set(mrb, self, MRB_IVSYM(minor_version), mrb_fixnum_value(minor_version));
      mrb_iv_set(mrb, self, MRB_IVSYM(headers), mrb_phr_headers_to_a(mrb, request_obj, headers, num_headers));

      return mrb_int_value(mrb, pret);
    }
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
  mrb_phr_t *mrb_phr = DATA_PTR(self);

  int pret = phr_parse_response(RSTRING_PTR(response_obj), RSTRING_LEN(response_obj),
      &minor_version, &status, &msg, &msg_len, headers,
      &num_headers, mrb_phr->last_len);

  switch (pret) {
    case -1: {
      return mrb_symbol_value(MRB_SYM(parser_error));
    } break;
    case -2: {
      mrb_phr->last_len = RSTRING_LEN(response_obj);
      return mrb_symbol_value(MRB_SYM(incomplete));
    } break;
    default: {
      mrb_iv_set(mrb, self, MRB_IVSYM(minor_version), mrb_fixnum_value(minor_version));
      mrb_iv_set(mrb, self, MRB_IVSYM(status), mrb_fixnum_value(status));
      mrb_iv_set(mrb, self, MRB_IVSYM(msg), mrb_str_substr(mrb, response_obj, msg - RSTRING_PTR(response_obj), msg_len));
      mrb_iv_set(mrb, self, MRB_IVSYM(headers), mrb_phr_headers_to_a(mrb, response_obj, headers, num_headers));

      return mrb_fixnum_value(pret);
    }
  }
}

static mrb_value
mrb_phr_parse_headers(mrb_state *mrb, mrb_value self)
{
  mrb_value headers_obj;

  mrb_get_args(mrb, "S", &headers_obj);

  struct phr_header headers[PHR_MAX_HEADERS];
  size_t num_headers = sizeof(headers) / sizeof(headers[0]);
  mrb_phr_t *mrb_phr = DATA_PTR(self);

  int pret = phr_parse_headers(RSTRING_PTR(headers_obj), RSTRING_LEN(headers_obj),
    headers, &num_headers, mrb_phr->last_len);

  switch (pret) {
    case -1: {
      return mrb_symbol_value(MRB_SYM(parser_error));
    } break;
    case -2: {
      mrb_phr->last_len = RSTRING_LEN(headers_obj);
      return mrb_symbol_value(MRB_SYM(incomplete));
    } break;
    default: {
      mrb_iv_set(mrb, self, MRB_IVSYM(headers), mrb_phr_headers_to_a(mrb, headers_obj, headers, num_headers));

      return mrb_fixnum_value(pret);
    }
  }
}

static mrb_value
mrb_phr_decode_chunked(mrb_state *mrb, mrb_value self)
{
  mrb_value buf;

  mrb_get_args(mrb, "S", &buf);
  mrb_str_modify(mrb, RSTRING(buf));
  mrb_phr_t *mrb_phr = DATA_PTR(self);

  size_t rsize = RSTRING_LEN(buf);
  int pret = phr_decode_chunked(&mrb_phr->decoder, RSTRING_PTR(buf), &rsize);

  switch (pret) {
    case -1: {
      return mrb_symbol_value(MRB_SYM(parser_error));
    } break;
    case -2: {
      return mrb_symbol_value(MRB_SYM(incomplete));
    } break;
    default: {
      mrb_str_resize(mrb, buf, rsize+pret);
      return mrb_fixnum_value(rsize);
    }
  }
}

static mrb_value
mrb_consume_trailer(mrb_state *mrb, mrb_value self)
{
  mrb_phr_t *mrb_phr = DATA_PTR(self);
  return mrb_bool_value(mrb_phr->decoder.consume_trailer);
}

static mrb_value
mrb_set_consume_trailer(mrb_state *mrb, mrb_value self)
{
  mrb_phr_t *mrb_phr = DATA_PTR(self);
  mrb_get_args(mrb, "b", &mrb_phr->decoder.consume_trailer);

  return self;
}

static mrb_value
mrb_phr_reset(mrb_state *mrb, mrb_value self)
{
  mrb_iv_remove(mrb, self, MRB_IVSYM(method));
  mrb_iv_remove(mrb, self, MRB_IVSYM(path));
  mrb_iv_remove(mrb, self, MRB_IVSYM(minor_version));
  mrb_iv_remove(mrb, self, MRB_IVSYM(status));
  mrb_iv_remove(mrb, self, MRB_IVSYM(msg));
  mrb_iv_remove(mrb, self, MRB_IVSYM(headers));
  memset(DATA_PTR(self), 0, sizeof(mrb_phr_t));

  return self;
}

void
mrb_mruby_phr_gem_init(mrb_state* mrb)
{
  struct RClass *phr_class = mrb_define_class_id(mrb, MRB_SYM(Phr), mrb->object_class);
  MRB_SET_INSTANCE_TT(phr_class, MRB_TT_DATA);
  mrb_define_method_id(mrb, phr_class, MRB_SYM(initialize),       mrb_phr_init,             MRB_ARGS_NONE());
  mrb_define_method_id(mrb, phr_class, MRB_SYM(parse_request),    mrb_phr_parse_request,    MRB_ARGS_REQ(1));
  mrb_define_method_id(mrb, phr_class, MRB_SYM(parse_response),   mrb_phr_parse_response,   MRB_ARGS_REQ(1));
  mrb_define_method_id(mrb, phr_class, MRB_SYM(parse_headers),    mrb_phr_parse_headers,    MRB_ARGS_REQ(1));
  mrb_define_method_id(mrb, phr_class, MRB_SYM(decode_chunked),   mrb_phr_decode_chunked,   MRB_ARGS_REQ(1));
  mrb_define_method_id(mrb, phr_class, MRB_SYM_Q(consume_trailer), mrb_consume_trailer,      MRB_ARGS_NONE());
  mrb_define_method_id(mrb, phr_class, MRB_SYM_E(consume_trailer), mrb_set_consume_trailer,  MRB_ARGS_REQ(1));
  mrb_define_method_id(mrb, phr_class, MRB_SYM(reset),            mrb_phr_reset,            MRB_ARGS_NONE());
}

void mrb_mruby_phr_gem_final(mrb_state* mrb) {}
