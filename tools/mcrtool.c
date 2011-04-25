#include "redstone.h"
#include <stdio.h>

int main(int argc, char** argv)
{
    if (argc != 2)
        return 1;
    
    RSRegion* reg = rs_region_open(argv[1]);
    rs_assert(reg);
    
    int x, z;
    for (z = 0; z < 32; z++)
    {
        for (x = 0; x < 32; x++)
        {
            if (rs_region_contains_chunk(reg, x, z))
            {
                printf("chunk size: %u time: %u\n", rs_region_get_chunk_size(reg, x, z), rs_region_get_chunk_timestamp(reg, x, z));
            }
        }
    }
    
    rs_region_close(reg);
	return 0;
}
