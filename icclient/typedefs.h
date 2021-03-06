#ifndef ICCLIENT_TYPEDEFS_H
#define ICCLIENT_TYPEDEFS_H

#include <stddef.h>
#ifdef __EMSCRIPTEN__
#include <emscripten/fetch.h>
typedef emscripten_fetch_t icclient_response;
#else
typedef struct {
	void *userData;
	char *data;
	size_t numBytes;
} icclient_response;
#endif

#endif
