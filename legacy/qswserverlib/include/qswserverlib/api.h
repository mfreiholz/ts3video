#ifndef SERVERCORE_API_HEADER
#define SERVERCORE_API_HEADER

#include <QtGlobal>

// DLL Export.
#ifdef SERVERCORE_EXPORT
#  define SERVERCORE_API Q_DECL_EXPORT
#else
#  define SERVERCORE_API
#endif

#endif