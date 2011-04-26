/*
 * This file is part of libredstone, and is distributed under the GNU LGPL.
 * See redstone.h for details.
 */

#ifndef __RS_COMPRESSION_H_INCLUDED__
#define __RS_COMPRESSION_H_INCLUDED__

#include "memory.h"

#include <stdint.h>

typedef enum
{
    RS_GZIP, /* RFC 1952 (usually unused) */
    RS_ZLIB, /* RFC 1950 */
    RS_UNKNOWN_COMPRESSION,
} RSCompressionType;

/* the return values (*outdata, *gzdata) MUST be freed! */
void rs_level_inflate(uint8_t* gzdata, size_t gzdatalen, uint8_t** outdata, size_t* outdatalen);
void rs_level_deflate(uint8_t* rawdata, size_t rawdatalen, uint8_t** gzdata, size_t* gzdatalen);

#endif /* __RS_COMPRESSION_H_INCLUDED__ */
