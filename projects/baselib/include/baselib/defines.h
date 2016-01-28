#ifndef BASELIB_DEFINES_H
#define BASELIB_DEFINES_H

#include <stdint.h>

// Defines from build system
#include "project-defines.h"

// Namespaces
#define OCS_NAMESPACE_BEGIN namespace ocs {
#define OCS_NAMESPACE_END }

OCS_NAMESPACE_BEGIN

// Basic types across all libraries
typedef int32_t clientid_t;
typedef int32_t channelid_t;

OCS_NAMESPACE_END

#endif