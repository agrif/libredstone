/*
 * This file is part of libredstone, and is distributed under the GNU LGPL.
 * See redstone.h for details.
 */

#include "compression.h"

#include "util.h"

#include <zlib.h>

/* nice, hefty 256kb buffers for storing uncompressed data */
#define RS_Z_BUFFER_SIZE (1024 * 256)
/* significantly less hefty 16kb buffers for compressed data */
#define RS_Z_SMALL_BUFFER_SIZE (1024 * 16)

void rs_level_inflate(uint8_t* gzdata, size_t gzdatalen, uint8_t** outdata, size_t* outdatalen)
{
    z_stream strm;
    int ret;
    
    /* initialize output to NULL, so we can check later
       also, this means error */
    *outdata = NULL;
    *outdatalen = 0;
    
    /* our output buffer */
    uint8_t output_buffer[RS_Z_BUFFER_SIZE];
    size_t output_write_head = 0;
    
    /* initialize zlib state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = gzdatalen;
    strm.next_in = gzdata;
    
    /* magick deflate init to handle gzip headers */
    ret = inflateInit2(&strm, 16 + MAX_WBITS);
    if (ret != Z_OK)
    {
        /* data could not be inflated */
        return;
    }
    
    /* loop continues until the output buffer is not full */
    do
    {
        /* set up the output buffer */
        strm.avail_out = RS_Z_BUFFER_SIZE;
        strm.next_out = output_buffer;
        
        /* inflate as much as possible */
        ret = inflate(&strm, Z_NO_FLUSH);
        
        /* make sure we're not clobbered */
        rs_assert(ret != Z_STREAM_ERROR);
        
        /* handle errors */
        switch (ret)
        {
        case Z_NEED_DICT:
        case Z_DATA_ERROR:
            /* level data is not valid gzip data */
            inflateEnd(&strm);
            if (*outdata)
            {
                rs_free(*outdata);
                *outdata = NULL;
                *outdatalen = 0;
            }
            return;
        case Z_MEM_ERROR:
            /* ran out of memory */
            inflateEnd(&strm);
            if (*outdata)
            {
                rs_free(*outdata);
                *outdata = NULL;
                *outdatalen = 0;
            }
            return;
        };
        
        size_t avail = RS_Z_BUFFER_SIZE - strm.avail_out;
        
        /* do something with output_buffer */
        
        if (*outdata == NULL)
        {
            *outdatalen = avail;
            *outdata = rs_malloc(avail);
            output_write_head = 0;            
        } else {
            *outdatalen += avail;
            *outdata = rs_realloc(*outdata, *outdatalen);
        }
        
        /* write the data after it */
        memcpy(outdata + output_write_head, output_buffer, avail);
        output_write_head += avail;
    } while (strm.avail_out == 0);
    
    /* free zlib info */
    inflateEnd(&strm);
    
    /* we ran out of input, so we better be at the end of the stream */
    if (ret != Z_STREAM_END)
    {
            /* level data is not valid gzip data */
            if (*outdata)
            {
                rs_free(*outdata);
                *outdata = NULL;
                *outdatalen = 0;
            }
            return;
    }
    
    /* our data is correct and properly placed, so... */
}

void rs_level_deflate(uint8_t* rawdata, size_t rawdatalen, uint8_t** gzdata, size_t* gzdatalen)
{
    z_stream strm;
    int ret;
    /* keep track of our flush state */
    int flush;
    
    /* initialize our output */
    *gzdata = NULL;
    *gzdatalen = 0;
    
    /* the buffer we read from -- data is copied in to it */
    uint8_t input_buffer[RS_Z_BUFFER_SIZE];
    /* wheere to copy from next in rawdata */
    size_t input_read_head = 0;
    
    /* temporary storage for gzip'd output */
    uint8_t output_buffer[RS_Z_SMALL_BUFFER_SIZE];
    
    /* initialize zlib state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    
    /* start deflating, with magick gzip arguments */
    /* RLE -- significantly faster, but very slightly less space efficient */
    /* compression level 1 gives OK results faster -- bandwidth is cheap, latency is not */
    /* FIXME configurable settings! */
    ret = deflateInit2(&strm, 1, Z_DEFLATED, 16 + MAX_WBITS, 8, Z_RLE);
    if (ret != Z_OK)
    {
        /* level data could not be deflated */
        return;
    }
    
    /* shovel in data until we run out */
    do
    {
        size_t copylen = MIN(RS_Z_BUFFER_SIZE, rawdatalen - input_read_head);
        memcpy(input_buffer, &rawdata[input_read_head], copylen);
        input_read_head += copylen;
        
        strm.avail_in = copylen;

        /* set up the input pointer */
        strm.next_in = input_buffer;
        
        /* set flush if this is the end */
        flush = (input_read_head == rawdatalen) ? Z_FINISH : Z_NO_FLUSH;
        
        /* now, deflate until there is no more output */
        do
        {
            /* set up output buffer */
            strm.next_out = output_buffer;
            strm.avail_out = RS_Z_SMALL_BUFFER_SIZE;
            
            /* deflate */
            ret = deflate(&strm, flush);
            
            /* make sure we're not corrupted */
            rs_assert(ret != Z_STREAM_ERROR);
            
            /* FIXME error checking? */
            
            /* available output */
            size_t avail = RS_Z_SMALL_BUFFER_SIZE - strm.avail_out;
            
            /* copy output buffer to the end of gzdata */
            *gzdata = rs_realloc(*gzdata, *gzdatalen + avail);
            memcpy(gzdata + *gzdatalen, output_buffer, avail);
            *gzdatalen += avail;
        } while (strm.avail_out == 0);
        
        /* be sure we used up all input */
        rs_assert(strm.avail_in == 0);
    } while (flush != Z_FINISH);
    
    /* we've finished the stream */
    rs_assert(ret == Z_STREAM_END);
    
    /* clean up zlib */
    deflateEnd(&strm);
}
