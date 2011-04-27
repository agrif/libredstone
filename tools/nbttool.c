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

const char* get_type_name(RSTagType type)
{
    switch (type)
    {
    case RS_TAG_END: return "TAG_End";
    case RS_TAG_BYTE: return "TAG_Byte";
    case RS_TAG_SHORT: return "TAG_Short";
    case RS_TAG_INT: return "TAG_Int";
    case RS_TAG_LONG: return "TAG_Long";
    case RS_TAG_FLOAT: return "TAG_Float";
    case RS_TAG_DOUBLE: return "TAG_Double";
    case RS_TAG_BYTE_ARRAY: return "TAG_Byte_Array";
    case RS_TAG_STRING: return "TAG_String";
    case RS_TAG_LIST: return "TAG_List";
    case RS_TAG_COMPOUND: return "TAG_Compound";
    }
    
    return "TAG_Unknown";
}

void print_indent(unsigned int indent)
{
    while (indent > 0)
    {
        printf("    ");
        indent--;
    }
}

void dump_tag(RSTag* tag, const char* name, unsigned int indent)
{
    print_indent(indent);
    printf("%s", get_type_name(rs_tag_get_type(tag)));
    if (name)
        printf("(\"%s\")", name);
    printf(": ");
    
    RSTagIterator it;
    const char* subname;
    RSTag* subtag;
    
    switch (rs_tag_get_type(tag))
    {
    case RS_TAG_END:
        printf("0\n");
        break;
    case RS_TAG_BYTE:
    case RS_TAG_SHORT:
    case RS_TAG_INT:
    case RS_TAG_LONG:
        printf("%li\n", rs_tag_get_integer(tag));
        break;
    case RS_TAG_FLOAT:
    case RS_TAG_DOUBLE:
        printf("%f\n", rs_tag_get_float(tag));
        break;
    case RS_TAG_BYTE_ARRAY:
        printf("%u bytes\n", rs_tag_get_byte_array_length(tag));
        break;
    case RS_TAG_STRING:
        printf("%s\n", rs_tag_get_string(tag));
        break;
    case RS_TAG_LIST:
        printf("%u entries of type %s\n", rs_tag_list_get_length(tag), get_type_name(rs_tag_list_get_type(tag)));
        print_indent(indent);
        printf("{\n");
        
        rs_tag_list_iterator_init(tag, &it);
        while (rs_tag_list_iterator_next(&it, &subtag))
        {
            dump_tag(subtag, NULL, indent + 1);
        }
        
        print_indent(indent);
        printf("}\n");
        break;
    case RS_TAG_COMPOUND:
        printf("%i entries\n", rs_tag_compound_get_length(tag));
        print_indent(indent);
        printf("{\n");
        
        rs_tag_compound_iterator_init(tag, &it);
        while (rs_tag_compound_iterator_next(&it, &subname, &subtag))
        {
            dump_tag(subtag, subname, indent + 1);
        }
        
        print_indent(indent);
        printf("}\n");
        break;
    default:
        printf("(unknown)\n");
        break;
    };
}

int main(int argc, char** argv)
{
    if (argc != 2 && argc != 4)
        return 1;
    
    RSNBT* nbt;
    if (argc == 2)
    {
        nbt = rs_nbt_open(argv[1]);
    } else {
        int x = atoi(argv[2]);
        int z = atoi(argv[3]);
        RSRegion* reg = rs_region_open(argv[1], false);
        rs_assert(reg);
        
        nbt = rs_nbt_parse_from_region(reg, x, z);
    }   
    rs_assert(nbt);
    
    dump_tag(rs_nbt_get_root(nbt), rs_nbt_get_name(nbt), 0);
    
    rs_nbt_free(nbt);
	return 0;
}
