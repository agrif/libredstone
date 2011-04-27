#include "redstone.h"
#include <stdio.h>

int main(int argc, char** argv)
{
    if (argc != 2)
        return 1;
    
    RSNBT* nbt = rs_nbt_new();
    
    rs_nbt_set_root(nbt, rs_tag_new(RS_TAG_COMPOUND));
    rs_nbt_set_name(nbt, "TestNBT");
    
    assert(rs_nbt_write_to_file(nbt, argv[1]));
    rs_nbt_free(nbt);
    
    return 0;
}
