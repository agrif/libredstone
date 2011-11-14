/*
 * This file is part of libredstone, and is distributed under the GNU LGPL.
 * See redstone.h for details.
 */

#ifndef __RS_REGION_H_INCLUDED__
#define __RS_REGION_H_INCLUDED__

/**
 * \defgroup region Region Reading and Writing
 *
 * This interface is responsible for reading data from and writing
 * data to Minecraft region files.
 *
 * It is not responsible for dealing with compression or parsing the
 * data that comes out of these files; for that, see the other parts
 * of libredstone.
 *
 * Note that right now region writing only works for brand-new region
 * files. In-place region writing is planned but not yet implemented.
 *
 * @{
 */

#include "compression.h"

#include <stdint.h>
#include <stdbool.h>

struct _RSRegion;
/**
 * The region data type.
 *
 * This is an opaque structure that acts as a handle, and is passed in
 * to all region-related functions.
 */
typedef struct _RSRegion RSRegion;

/**
 * Open the given region file.
 *
 * This function opens the file at the given path, and attempts to
 * parse it as a region file. If it is successful, it will return a
 * new RSRegion handle. If not, it will return NULL.
 *
 * \param path the path to the region file
 * \param write whether to open the file with write mode or not
 * \return the new region object, or NULL
 * \sa rs_region_close
 */
RSRegion* rs_region_open(const char* path, bool write);

/**
 * Close the given region file.
 *
 * Use this function when you are done with a region file. It closes
 * the file and frees all memory associated with the region object.
 *
 * Before it closes the file, it calls rs_region_flush() to write any
 * cached chunk data.
 *
 * \param self the region to close
 * \sa rs_region_open
 */
void rs_region_close(RSRegion* self);

/**
 * Get the last modified time of a given chunk.
 *
 * This retrieves the modified time of the chunk at the coordinates
 * given. If the chunk does not exist, this is usually (but not
 * always) 0 -- it depends on how sane Minecraft was feeling when it
 * wrote the region file.
 *
 * \param self the region file
 * \param x the x coordinate of the chunk
 * \param z the z coordinate of the chunk
 * \return the last modified time of the chunk in question
 */
uint32_t rs_region_get_chunk_timestamp(RSRegion* self, uint8_t x, uint8_t z);

/**
 * Get the length of the chunk data.
 *
 * This retrieves the length of the data stored in this region file
 * for the given chunk. If the chunk does not exist, this is 0.
 *
 * If you want to read the data for a given chunk, use this in
 * combination with rs_region_get_chunk_data().
 *
 * \param self the region file
 * \param x the x coordinate of the chunk
 * \param z the z coordinate of the chunk
 * \return the length of the data for the chunk in question
 * \sa rs_region_get_chunk_data
 */
uint32_t rs_region_get_chunk_length(RSRegion* self, uint8_t x, uint8_t z);

/**
 * Get the compression type of the chunk data.
 *
 * Chunk data can be (hypothetically) stored in many different
 * compression formats, though it is almost always RS_ZLIB. Use this
 * function to figure out for sure.
 *
 * \param self the region file
 * \param x the x coordinate of the chunk
 * \param z the z coordinate of the chunk
 * \return the compression type used for the chunk in question
 * \sa rs_region_get_chunk_data
 */
RSCompressionType rs_region_get_chunk_compression(RSRegion* self, uint8_t x, uint8_t z);

/**
 * Get the data for a chunk.
 *
 * This function returns a pointer to the chunk data at the given
 * coordinates. This pointer is valid until the region is closed, or
 * until rs_region_flush() is called.
 *
 * Use this in combination with rs_region_get_chunk_length().
 *
 * \param self the region file
 * \param x the x coordinate of the chunk
 * \param z the z coordinate of the chunk
 * \return the data stored for the given chunk
 * \sa rs_region_get_chunk_length, rs_region_get_chunk_compression
 */
void* rs_region_get_chunk_data(RSRegion* self, uint8_t x, uint8_t z);

/**
 * Get whether a chunk is present.
 *
 * This is a convenience function that easily performs all the checks
 * for chunk existance in a region file.
 *
 * \param self the region file
 * \param x the x coordinate of the chunk
 * \param z the z coordinate of the chunk
 * \return true if the region contains the given chunk, false otherwise
 */
#define rs_region_contains_chunk(self, x, z) (rs_region_get_chunk_timestamp((self), (x), (z)) == 0 || (int32_t)rs_region_get_chunk_length((self), (x), (z)) <= 0 ? false : true)

/**
 * Set the data for a given chunk.
 *
 * This function acts identically to rs_region_set_chunk_data_full(),
 * except it automatically sets the modification time to the current
 * time. All the caveats and usage information still apply.
 *
 * \param self the region file
 * \param x the x coordinate of the chunk
 * \param z the z coordinate of the chunk
 * \param data the data to use
 * \param len the length of the data
 * \param enc the compression type used on data, or RS_AUTO_COMPRESSION
 * \sa rs_region_set_chunk_data_full
 * \sa rs_region_clear_chunk
 * \sa rs_region_flush
 */
void rs_region_set_chunk_data(RSRegion* self, uint8_t x, uint8_t z, void* data, uint32_t len, RSCompressionType enc);

/**
 * Set the data and modification time for a given chunk.
 *
 * This is the primary interface for writing to region files. Note
 * that the data is copied and stored until the region is closed or
 * until rs_region_flush() is called. Since it is copied, you are not
 * responsible for holding on to the data yourself; it may be freed
 * immediately after this call.
 *
 * This call will only work if the region was opened in write mode.
 *
 * If the given compression type is RS_AUTO_COMPRESSION, the
 * compression type will be guessed from the given data.
 *
 * See rs_region_set_chunk_data() for a version that automatically
 * sets the modification time to the current time. If you want to
 * delete a chunk instead, use rs_region_clear_chunk().
 *
 * \param self the region file
 * \param x the x coordinate of the chunk
 * \param z the z coordinate of the chunk
 * \param data the data to use
 * \param len the length of the data
 * \param enc the compression type used on data, or RS_AUTO_COMPRESSION
 * \param timestamp the modification time to use
 * \sa rs_region_set_chunk_data
 * \sa rs_region_clear_chunk
 * \sa rs_region_flush
 */
void rs_region_set_chunk_data_full(RSRegion* self, uint8_t x, uint8_t z, void* data, uint32_t len, RSCompressionType enc, uint32_t timestamp);

/**
 * Delete the given chunk.
 *
 * Use this to delete the given chunk from the region file. This will
 * only work if the region was opened in write mode.
 *
 * Chunk deletes are cached alongside chunk writes, and will not take
 * effect until the region is closed, or rs_region_flush() is called.
 *
 * \param self the region file
 * \param x the x coordinate of the chunk
 * \param z the z coordinate of the chunk
 * \sa rs_region_set_chunk_data
 * \sa rs_region_flush
 */
void rs_region_clear_chunk(RSRegion* self, uint8_t x, uint8_t z);

/**
 * Flush the cached writes, and reread the file.
 *
 * This function flushes all cached writes, and rereads the region
 * file. If the region was not opened in write mode, this simply
 * rereads the file.
 *
 * As a consequence, all existing chunk data pointers are invalidated.
 *
 * \param self the region to flush
 * \sa rs_region_set_chunk_data
 * \sa rs_region_clear_chunk
 */
void rs_region_flush(RSRegion* self);

/** @} */
#endif /* __RS_REGION_H_INCLUDED__ */
