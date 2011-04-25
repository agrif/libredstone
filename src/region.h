#ifndef __RS_REGION_H_INCLUDED__
#define __RS_REGION_H_INCLUDED__

#include <stdint.h>
#include <stdbool.h>

struct _RSRegion;
typedef struct _RSRegion RSRegion;

typedef enum
{
    RS_GZIP, /* RFC 1952 (usually unused) */
    RS_ZLIB, /* RFC 1950 */
    RS_UNKNOWN_COMPRESSION,
} RSCompressionType;

/* opening/closing */
RSRegion* rs_region_open(const char* path);
void rs_region_close(RSRegion* self);

/* low-level chunk access */
uint32_t rs_region_get_chunk_timestamp(RSRegion* self, uint8_t x, uint8_t z);
uint32_t rs_region_get_chunk_size(RSRegion* self, uint8_t x, uint8_t z);
RSCompressionType rs_region_get_chunk_compression(RSRegion* self, uint8_t x, uint8_t z);
/* valid until region is closed */
void* rs_region_get_chunk_data(RSRegion* self, uint8_t x, uint8_t z);

#define rs_region_contains_chunk(self, x, z) (rs_region_get_chunk_timestamp((self), (x), (z)) == 0 ? false : true)

#endif /* __RS_REGION_H_INCLUDED__ */
