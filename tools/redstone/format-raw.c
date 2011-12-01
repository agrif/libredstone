/*
 * This program is part of libredstone.
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "formats.h"

static void raw_dump(RSToolOptions* opts, RSNBT* nbt, FILE* out)
{
    void* data;
    size_t length;
    rs_nbt_write(nbt, &data, &length, RS_GZIP);
    
    fwrite(data, 1, length, out);
    
    rs_free(data);
}

static RSNBT* raw_load(RSToolOptions* opts, FILE* in)
{
    void* buffer = NULL;
    size_t buf_size = 1024 * 32;
    size_t buf_used = 0;
    buffer = rs_malloc(buf_size);
    
    while(true)
    {
        rs_assert(buf_used < buf_size);
        buf_used += fread(buffer, 1, buf_size - buf_used, in);
        if (buf_used != buf_size)
        {
            /* we're done! */
            break;
        }
        
        /* still more to read */
        buf_size *= 2;
        buffer = rs_realloc(buffer, buf_size);
    }

    RSNBT* nbt = rs_nbt_parse(buffer, buf_used, RS_AUTO_COMPRESSION);
    
    /* free the buffer */
    rs_free(buffer);
    
    if (!nbt)
        opts->error("could not parse raw NBT input\n");
    return nbt;
}

RSToolFormatter rs_tool_formatter_raw = {
    .name = "raw",
    .description = "standalone, gzip'd raw NBT (like level.dat)",
    .dump = raw_dump,
    .load = raw_load,
};
