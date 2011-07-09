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

#include "redstone.h"
#include <stdio.h>

#define INSIDE_EXMAPLE(x, z) ((x) >= 11 && (x) <= 22 && (z) >= 1 && (z) <= 10)

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "usage: %s [input] [output]\n", argv[0]);
        return 1;
    }
    
    RSRegion* reg = rs_region_open(argv[1], false);
    RSRegion* out = rs_region_open(argv[2], true);
    rs_assert(reg);
    rs_assert(out);

    int x = 0, z = 0;
    for (z = 0; z < 32; z++)
    {
        for (x = 0; x < 32; x++)
        {
            if (INSIDE_EXMAPLE(x, z) && rs_region_contains_chunk(reg, x, z))
            {
                void* data = rs_region_get_chunk_data(reg, x, z);
                uint32_t len = rs_region_get_chunk_length(reg, x, z);
                RSCompressionType comp = rs_region_get_chunk_compression(reg, x, z);
                uint32_t timestamp = rs_region_get_chunk_timestamp(reg, x, z);
                
                rs_region_set_chunk_data_full(out, x, z, data, len, comp, timestamp);
            }
        }
    }
    
    rs_region_flush(out);
    rs_region_close(out);
    rs_region_close(reg);

	return 0;
}
