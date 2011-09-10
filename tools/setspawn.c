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

int main(int argc, char** argv)
{
    if (argc != 5)
    {
        if (argc != 1)
            fprintf(stderr, "invalid number of arguments\n");
        fprintf(stderr, "Usage: %s <level.dat> <X> <Y> <Z>\n", argv[0]);
        return 1;
    }
    
    char* endptr;
    
    long x = strtol(argv[2], &endptr, 10);
    if (*endptr != 0)
    {
        fprintf(stderr, "X value not an integer: `%s'\n", argv[2]);
        return 1;
    }

    long y = strtol(argv[3], &endptr, 10);
    if (*endptr != 0)
    {
        fprintf(stderr, "Y value not an integer: `%s'\n", argv[3]);
        return 1;
    }

    long z = strtol(argv[4], &endptr, 10);
    if (*endptr != 0)
    {
        fprintf(stderr, "Z value not an integer: `%s'\n", argv[4]);
        return 1;
    }
    
    RSNBT* nbt;
    nbt = rs_nbt_parse_from_file(argv[1]);
    
    if (!nbt)
    {
        fprintf(stderr, "could not load NBT file: `%s'\n", argv[1]);
        return 1;
    }
    
    RSTag* xtag = rs_nbt_find(nbt, "SpawnX");
    RSTag* ytag = rs_nbt_find(nbt, "SpawnY");
    RSTag* ztag = rs_nbt_find(nbt, "SpawnZ");
    if (!xtag || rs_tag_get_type(xtag) != RS_TAG_INT ||
        !ytag || rs_tag_get_type(ytag) != RS_TAG_INT ||
        !ztag || rs_tag_get_type(ztag) != RS_TAG_INT)
    {
        fprintf(stderr, "invalid level.dat: `%s'\n", argv[1]);
        rs_nbt_free(nbt);
        return 1;
    }
    
    rs_tag_set_integer(xtag, x);
    rs_tag_set_integer(ytag, y);
    rs_tag_set_integer(ztag, z);
    
    if (!rs_nbt_write_to_file(nbt, argv[1]))
    {
        fprintf(stderr, "could not write to file: `%s'\n", argv[1]);
        rs_nbt_free(nbt);
        return 1;
    }
    
    fprintf(stdout, "Spawn successfully set.\n");
    
    rs_nbt_free(nbt);
	return 0;
}
