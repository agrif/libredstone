#include "redstone.h"
#include <stdio.h>

int main(int argc, char** argv)
{
    if (argc != 3)
        return 1;
    
    RSRegion* reg = rs_region_open(argv[1], false);
    rs_assert(reg);

    RSRegion* write = rs_region_open(argv[2], true);
    rs_assert(write);
    
    int x = 0, z = 0;
    for (z = 0; z < 32; z++)
    {
        for (x = 0; x < 32; x++)
        {
            if (rs_region_contains_chunk(reg, x, z))
            {
                break;
            }
        }
        if (rs_region_contains_chunk(reg, x, z))
            break;
    }
    
    int dx, dz;
    for (dz = 0; dz < 32; dz++)
    {
        for (dx = 0; dx < 32; dx++)
        {
            rs_region_set_chunk_data(write, dx, dz,
                                     rs_region_get_chunk_data(reg, x, z),
                                     rs_region_get_chunk_length(reg, x, z),
                                     rs_region_get_chunk_compression(reg, x, z));
        }
    }

    printf("writing...\n");
    rs_region_close(write);
    rs_region_close(reg);
    printf("done.\n");
    
	return 0;
}
