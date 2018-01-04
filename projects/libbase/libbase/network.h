#pragma once

#ifdef _WIN32
	#include <WinSock2.h>
#elif __APPLE__
	#include <netinet/in.h>
#elif __linux__
	#include <netinet/in.h>
	#define htonll(x) ((1==htonl(1)) ? (x) : ((uint64_t)htonl((x) & 0xFFFFFFFF) << 32) | htonl((x) >> 32))
	#define ntohll(x) ((1==ntohl(1)) ? (x) : ((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32))
#endif

#if defined(__APPLE__) || defined(__linux__)
	//  Nothing yet.
#endif
