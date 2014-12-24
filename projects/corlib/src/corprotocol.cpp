#if defined(_WIN32)
#include <WinSock2.h>
#elif defined(__linux__)
#include <netinet/in.h>
#define htonll(x) x
#define ntohll(x) x
#endif
#include <cstdlib>
#include <cstring>
#include "corprotocol.h"

///////////////////////////////////////////////////////////////////////
// COR FRAME
///////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////
// PARSER
///////////////////////////////////////////////////////////////////////

#define EXECUTE_CALLBACK_1(cbfunc, P) if (cbfunc && P) { cbfunc(P); }
#define EXECUTE_CALLBACK_1_COND(cond, cbfunc, P) if (cond) { EXECUTE_CALLBACK_1(cbfunc, P) }

#define EXECUTE_CALLBACK_3(cbfunc, P, ARG1, ARG2) if (cbfunc && P) { cbfunc(P, ARG1, ARG2); }

enum state {
  s_none,

  s_header_begin,
  s_header_version,
  s_header_type,
  s_header_flags,
  s_header_correlation_id,
  s_header_body_length,
  s_header_end,
  
  s_body_data
};

void cor_parser_settings_init(cor_parser_settings &sett)
{
  sett.on_frame_begin = 0;
  sett.on_frame_header_begin = 0;
  sett.on_frame_header_end = 0;
  sett.on_frame_body_data_begin = 0;
  sett.on_frame_body_data = 0;
  sett.on_frame_body_data_end = 0;
  sett.on_frame_end = 0;
}

void cor_parser_init(cor_parser *parser)
{
  parser->state = s_none;
  parser->request_body_bytes_read = 0;
  parser->request = 0;
}

size_t cor_parser_parse(cor_parser *parser, const cor_parser_settings &settings, const uint8_t *data, size_t length)
{
  const uint8_t *p = data;
  size_t readBytes = 0;
  if (!p || length <= 0) {
    return 0;
  }

  while (readBytes < length) {
    // Initialize new request.
    switch (parser->state) {
      case s_none:
        parser->state = s_header_version;
        parser->request = (cor_frame*)malloc(sizeof(cor_frame));
        EXECUTE_CALLBACK_1(settings.on_frame_begin, parser);
        EXECUTE_CALLBACK_1(settings.on_frame_header_begin, parser);
        continue;
    }
    // Parse header.
    if (parser->state > s_header_begin && parser->state < s_header_end) {
      size_t fieldSize = 0;
      void *fieldValueDst = 0;
      state nextState = s_none;
      switch (parser->state) {
        case s_header_version:
          fieldSize = sizeof(cor_frame::version_t);
          fieldValueDst = &parser->request->version;
          nextState = s_header_type;
          break;
        case s_header_type:
          fieldSize = sizeof(cor_frame::type_t);
          fieldValueDst = &parser->request->type;
          nextState = s_header_flags;
          break;
        case s_header_flags:
          fieldSize = sizeof(cor_frame::flags_t);
          fieldValueDst = &parser->request->flags;
          nextState = s_header_correlation_id;
          break;
        case s_header_correlation_id:
          fieldSize = sizeof(cor_frame::correlation_t);
          fieldValueDst = &parser->request->correlation_id;
          nextState = s_header_body_length;
          break;
        case s_header_body_length:
          fieldSize = sizeof(cor_frame::data_length_t);
          fieldValueDst = &parser->request->length;
          nextState = s_body_data;
          break;
      }
      if (length - readBytes < fieldSize) {
        return readBytes;
      }

      if (fieldSize == 2) {
        uint16_t val;
        memcpy(&val, p, fieldSize);
        val = ntohs(val);
        memcpy(fieldValueDst, &val, fieldSize);
      } else if (fieldSize == 4) {
        uint32_t val = 0;
        memcpy(&val, p, fieldSize);
        val = ntohl(val);
        memcpy(fieldValueDst, &val, fieldSize);
      } else if (fieldSize == 8) {
        uint64_t val = 0;
        memcpy(&val, p, fieldSize);
        val = ntohll(val);
        memcpy(fieldValueDst, &val, fieldSize);
      }
      parser->state = nextState;
      p += fieldSize;
      readBytes += fieldSize;

      // TODO Convert byte order.
      //memcpy(fieldValueDst, p, fieldSize);
      //parser->state = nextState;
      //p += fieldSize;
      //readBytes += fieldSize;
      
      EXECUTE_CALLBACK_1_COND(parser->state > s_header_end, settings.on_frame_header_end, parser);
      continue;
    }

    // Body data.
    if (parser->state == s_body_data &&  parser->request_body_bytes_read < parser->request->length) {
      const size_t lengthLeft = length - readBytes;
      const size_t requestBodyDataLeft = parser->request->length - parser->request_body_bytes_read;
      size_t len = lengthLeft;
      if (lengthLeft > requestBodyDataLeft) {
        len = requestBodyDataLeft;
      }
      EXECUTE_CALLBACK_3(settings.on_frame_body_data, parser, p, len);
      p += len;
      readBytes += len;
      parser->request_body_bytes_read += len;
    }

    // Check for end of body data.
    if (parser->state == s_body_data && parser->request_body_bytes_read == parser->request->length) {
      EXECUTE_CALLBACK_1(settings.on_frame_body_data_end, parser);
      EXECUTE_CALLBACK_1(settings.on_frame_end, parser);
      free(parser->request);
      cor_parser_init(parser);
    }

  } // while
  return readBytes;
}
