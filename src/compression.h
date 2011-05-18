/*
 * This file is part of libredstone, and is distributed under the GNU LGPL.
 * See redstone.h for details.
 */

#ifndef __RS_COMPRESSION_H_INCLUDED__
#define __RS_COMPRESSION_H_INCLUDED__

/**
 * \defgroup compression Compression Utilities
 *
 * These functions provide compression and decompression throughout
 * libredstone, through a variety of different algorithms. There is
 * also a function to guess what compression algorithm a given set of
 * data was compressed with.
 *
 * @{
 */

#include "memory.h"

#include <stdint.h>

/**
 * Supported compression methods.
 *
 * This enumeration is used all over libredstone whenever a
 * compression type is required. If you don't know the type, use
 * RS_AUTO_COMPRESSION and the type will be guessed automatically.
 */
typedef enum
{
    /**
     * Use this for almost everything -- it means "figure it out based
     * on the data given", and works very well. Receiving functions
     * determine type via rs_get_compression_type().
     */
    RS_AUTO_COMPRESSION,
    
    /**
     * RFC 1952 (for standalone NBT)
     */
    RS_GZIP,
    
    /**
     * RFC 1950 (for region file NBT)
     */
    RS_ZLIB,
    
    /**
     * Used by rs_get_compression_type() to indicate unknown type.
     */
    RS_UNKNOWN_COMPRESSION,
} RSCompressionType;

/**
 * Decompress the given data.
 *
 * Use this to conveniently decompress the given data. Decompressed
 * data will be written to a newly-allocated buffer in *outdata, and
 * the length of this data will be written to *outdatalen. The buffer
 * stored in *outdata MUST be freed!
 *
 * In the event of an error, *outdata will be set to NULL.
 *
 * \param enc the type of encoding to decompress with
 * \param gzdata the data to decompress
 * \param gzdatalen the length of gzdata
 * \param outdata where to store the output buffer
 * \param outdatalen where to store the output buffer length
 * \sa rs_compress, rs_free
 */
void rs_decompress(RSCompressionType enc, uint8_t* gzdata, size_t gzdatalen, uint8_t** outdata, size_t* outdatalen);

/**
 * Compress the given data.
 *
 * Use this to conveniently compress the given data. Compressed data
 * will be written to a newly-allocated buffer in *gzdata, and the
 * length of this data will be written to *gzdatalen. The buffer
 * stored in *gzdata MUST be freed!
 *
 * In the event of an error, *gzdata will be set to NULL.
 *
 * \param enc the type of encoding to compress with
 * \param rawdata the data to compress
 * \param rawdatalen the length of rawdata
 * \param gzdata where to store the compressed output buffer
 * \param gzdatalen where to store the compressed output buffer length
 * \sa rs_decompress, rs_free
 */
void rs_compress(RSCompressionType enc, uint8_t* rawdata, size_t rawdatalen, uint8_t** gzdata, size_t* gzdatalen);

/**
 * Use this to intelligently guess compression type.
 *
 * The data you pass in must be at least a few bytes long. If the
 * compression type cannot be guessed, it will return
 * RS_UNKNOWN_COMPRESSION.
 *
 * \param data the data to guess for
 * \param len the length of the data
 * \return the guessed compression type
 */
RSCompressionType rs_get_compression_type(void* data, size_t len);

/** @} */
#endif /* __RS_COMPRESSION_H_INCLUDED__ */
