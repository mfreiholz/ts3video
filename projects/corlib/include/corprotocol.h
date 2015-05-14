#ifndef COR_PROTOCOL_H
#define COR_PROTOCOL_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint16_t  cor_version_t;
typedef uint16_t  cor_type_t;
typedef uint8_t   cor_flags_t;
typedef uint32_t  cor_correlation_t;
typedef uint32_t  cor_data_length_t;

static const size_t COR_FRAME_SIZE = 
    sizeof(cor_version_t) + 
    sizeof(cor_type_t) +
    sizeof(cor_flags_t) +
    sizeof(cor_correlation_t) +
    sizeof(cor_data_length_t);
    
static const cor_type_t COR_FRAME_TYPE_REQUEST = 0;
static const cor_type_t COR_FRAME_TYPE_RESPONSE = 1;

typedef struct cor_frame
{
  cor_version_t version;             ///< (16-bit) The version number of the protocol.
  cor_type_t type;                   ///< (16-bit) The type of the request (0=request, 1=response)
  cor_flags_t flags;                 ///< (8-bit)  Custom flags (Can be different, based on the type.)
  cor_correlation_t correlation_id;  ///< (32-bit) Correlation ID which associates an request and response.
  cor_data_length_t length;          ///< (32-bit) Length of the upcoming data block.
  uint8_t *data;                 ///< (bytes)  The actual frame data.
} cor_frame;

#ifdef __cplusplus
}
#endif

#endif
