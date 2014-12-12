#ifndef TCPPROTOCOL_HEADER
#define TCPPROTOCOL_HEADER

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////
// COR protocol objects
///////////////////////////////////////////////////////////////////////

typedef struct
{
  typedef uint16_t  version_t;
  typedef uint16_t  type_t;
  typedef uint8_t   flags_t;
  typedef uint32_t  correlation_t;
  typedef uint32_t  data_length_t;

  version_t version;             ///< (16-bit) The version number of the protocol.
  type_t type;                   ///< (16-bit) The type of the request (0=request, 1=response)
  flags_t flags;                 ///< (8-bit)  Custom flags (Can be different, based on the type.)
  correlation_t correlation_id;  ///< (32-bit) Correlation ID which associates an request and response.
  data_length_t length;          ///< (32-bit) Length of the upcoming data block.
  uint8_t *data;                 ///< (bytes)  The actual frame data.

  static const size_t MINSIZE = sizeof(version_t) + sizeof(type_t) + sizeof(flags_t) + sizeof(correlation_t) + sizeof(data_length_t);
  static const type_t TYPE_REQUEST = 0;
  static const type_t TYPE_RESPONSE = 1;
} cor_frame;

///////////////////////////////////////////////////////////////////////
// PARSER
///////////////////////////////////////////////////////////////////////

typedef struct cor_parser cor_parser;
typedef struct cor_parser_settings cor_parser_settings;

// Callback declarations.
typedef int (*cor_parser_cb) (cor_parser *parser);
typedef int (*cor_parser_data_cb) (cor_parser *parser, const uint8_t *data, size_t length);

struct cor_parser
{
  // PRIVATE
  uint32_t state; ///< Current parser state from *.c
  size_t request_body_bytes_read; ///< Number of bytes that has been read of the current cor_frame body.

  // READ ONLY
  cor_frame *request; ///< Current cor_frame being parsed.

  // PUBLIC
  void *object; ///< Pointer to the custom caller object.
};

struct cor_parser_settings
{
  // CALLBACKS
  cor_parser_cb on_frame_begin;
  cor_parser_data_cb on_frame_body_data;
  cor_parser_cb on_frame_end;
};

/* Initializes/resets an parser object.
 */
void cor_parser_init(cor_parser *parser);

/* Executes parsing.
 * @return Number of parsed bytes.
 */
size_t cor_parser_parse(cor_parser *parser, const cor_parser_settings &settings, const uint8_t *data, size_t len);

#ifdef __cplusplus
}
#endif

#endif