#include "redstone.h"
#include <stdio.h>

int main(int argc, char** argv)
{
    if (argc != 2)
        return 1;
    
    RSNBT* nbt = rs_nbt_new();
    
    RSTag* root = rs_tag_new(RS_TAG_COMPOUND,
                             "list", rs_tag_new(RS_TAG_LIST,
                                                rs_tag_new(RS_TAG_SHORT, 42),
                                                rs_tag_new(RS_TAG_SHORT, 43),
                                                rs_tag_new(RS_TAG_SHORT, 44),
                                                NULL),
                             "compound", rs_tag_new(RS_TAG_COMPOUND,
                                                    "int", rs_tag_new(RS_TAG_INT, 1337),
                                                    "long", rs_tag_new(RS_TAG_LONG, 100000),
                                                    "float", rs_tag_new(RS_TAG_FLOAT, 1.337),
                                                    "double", rs_tag_new(RS_TAG_DOUBLE, 1.33337),
                                                    NULL),
                             "byte array", rs_tag_new(RS_TAG_BYTE_ARRAY, 5, "whelp"),
                             "byte", rs_tag_new(RS_TAG_BYTE, 0),
                             NULL);
    
    rs_nbt_set_root(nbt, root);
    rs_nbt_set_name(nbt, "TestNBT");
    
    assert(rs_nbt_write_to_file(nbt, argv[1]));
    rs_nbt_free(nbt);
    
    return 0;
}
