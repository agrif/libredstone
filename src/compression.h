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
    /* use this for almost everything -- it means "figure it out based
     * on the data given", and works very well
     * receiving functions will determine type via rs_get_compression_type()
     */
    RS_AUTO_COMPRESSION,
    
    /* more specific types */
    
    RS_GZIP, /* RFC 1952 (for standalone NBT) */
    RS_ZLIB, /* RFC 1950 (for region file NBT) */
    RS_UNKNOWN_COMPRESSION,
} RSCompressionType;

/* the return values (*outdata, *gzdata) MUST be freed! */
/* also, gzdata is just a variable name, returned data may not be GZIP'd */
void rs_decompress(RSCompressionType enc, uint8_t* gzdata, size_t gzdatalen, uint8_t** outdata, size_t* outdatalen);
void rs_compress(RSCompressionType enc, uint8_t* rawdata, size_t rawdatalen, uint8_t** gzdata, size_t* gzdatalen);

/* use this to intelligently guess compression type. the data you pass
 * in must be at least a few bytes long
 */
RSCompressionType rs_get_compression_type(void* data, size_t len);

#endif /* __RS_COMPRESSION_H_INCLUDED__ */
