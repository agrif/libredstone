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
    if (argc != 2 && argc != 4)
        return 1;
    
    RSNBT* nbt;
    if (argc == 2)
    {
        nbt = rs_nbt_parse_from_file(argv[1]);
    } else {
        int x = atoi(argv[2]);
        int z = atoi(argv[3]);
        RSRegion* reg = rs_region_open(argv[1], false);
        rs_assert(reg);
        
        nbt = rs_nbt_parse_from_region(reg, x, z);
        rs_region_close(reg);
    }   
    rs_assert(nbt);
    
    rs_tag_pretty_print(rs_nbt_get_root(nbt), stdout);
    
    rs_nbt_free(nbt);
	return 0;
}
