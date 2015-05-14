#ifndef COR_PARSER_H
#define COR_PARSER_H

#include "corprotocol.h"

#ifdef __cplusplus
extern "C" {
#endif

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
  cor_parser_cb on_frame_header_begin;
  cor_parser_cb on_frame_header_end;
  cor_parser_cb on_frame_body_data_begin;
  cor_parser_data_cb on_frame_body_data;
  cor_parser_cb on_frame_body_data_end;
  cor_parser_cb on_frame_end;
};

/* Initializes parser settings.
 */
void cor_parser_settings_init(cor_parser_settings *sett);

/* Initializes/resets an parser object.
 */
void cor_parser_init(cor_parser *parser);

/* Executes parsing.
 * @return Number of parsed bytes.
 */
size_t cor_parser_parse(cor_parser *parser, const cor_parser_settings *settings, const uint8_t *data, size_t len);

#ifdef __cplusplus
}
#endif

#endif
