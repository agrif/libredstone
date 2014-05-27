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
#include "config.h"
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#if HAVE_MKDIR
#  if MKDIR_TAKES_ONE_ARG
#    define mkdir(a, b) mkdir(a)
#  endif
#else
#  if HAVE__MKDIR
#    define mkdir(a, b) _mkdir(a)
#  else
#    error "Don't know how to create a directory on this system."
#  endif
#endif

#define index_block(blocks, x, y, z) ((blocks)[y + (z) * 128 + (x) * 128 * 16])
#define set_half_byte(dest, x, y, z, val)                               \
    do                                                                  \
    {                                                                   \
        uint16_t index = ((y) + (z) * 128 + (x) * 128 * 16) / 2;        \
        if ((y) % 2 == 0)                                               \
        {                                                               \
            dest[index] &= ~0x0F;                                       \
            dest[index] |= val & 0x0F;                                  \
        } else {                                                        \
            dest[index] &= ~0xF0;                                       \
            dest[index] |= (val << 4) & 0xF0;                           \
        }                                                               \
    } while (0)

void generate_terrain(int cx, int cz, uint8_t* blocks)
{
    int x, y, z;
    
    for (x = 0; x < 16; x++)
    {
        for (z = 0; z < 16; z++)
        {
            for (y = 0; y < 128; y++)
            {
                int local_x = cx * 16 + x;
                int local_z = cz * 16 + z;
                int local_y = y * 4 - 256;
                
                if (local_x < 0)
                    local_x *= -1;
                if (local_z < 0)
                    local_z *= -1;
                local_x -= 256;
                local_z -= 256;
                
                if (y < 64)
                    index_block(blocks, x, y, z) = 3;
                if (y == 64)
                    index_block(blocks, x, y, z) = 2;
                
                if (local_x * local_x + local_y * local_y + local_z * local_z < 240 * 240)
                {
                    index_block(blocks, x, y, z) = 1;
                    if (local_x * local_x + local_y * local_y + local_z * local_z < 200 * 200)
                    {
                        index_block(blocks, x, y, z) = 0;
                    } else {
                        float orerand = (float)rand()/(float)RAND_MAX;
                        if (orerand < 0.01)
                        {
                            index_block(blocks, x, y, z) = 56;
                        } else if (orerand < 0.05) {
                            index_block(blocks, x, y, z) = 14;
                        } else if (orerand < 0.12) {
                            index_block(blocks, x, y, z) = 15;
                        } else if (orerand < 0.3) {
                            index_block(blocks, x, y, z) = 16;
                        }
                    }
                }
                
                if (((local_x <= 1 && local_x >= 0) || (local_z <= 1 && local_z >= 0)) && (y >= 64 && y < 68))
                    index_block(blocks, x, y, z) = (y == 64 ? 17 : 0);
            }
        }
    }
}

void generate_heightmap(uint8_t* blocks, uint8_t* heightmap)
{
    int x, y, z;
    for (x = 0; x < 16; x++)
    {
        for (z = 0; z < 16; z++)
        {
            for (y = 127; y >= 0; y--)
            {
                if (index_block(blocks, x, y, z) != 0)
                    break;
            }
            /* note weird index ordering */
            heightmap[z + 16 * x] = y + 1;
        }
    }
}

void generate_skylight(uint8_t* blocks, uint8_t* heightmap, uint8_t* skylight)
{
    int x, y, z;
    for (x = 0; x < 16; x++)
    {
        for (z = 0; z < 16; z++)
        {
            uint8_t height = heightmap[z + 16 * x];
            for (y = 0; y < 128; y++)
            {
                if (y >= height)
                {
                    set_half_byte(skylight, x, y, z, 0x0F);
                }
            }
        }
    }
}

RSTag* create_chunk(int x, int z, uint8_t* zero_height)
{
    uint8_t* blocks = rs_malloc0(0x8000);
    uint8_t* blocklight = rs_malloc0(0x4000);
    uint8_t* skylight = rs_malloc0(0x4000);
    uint8_t* data = rs_malloc0(0x4000);
    uint8_t* heightmap = rs_malloc0(0x100);
    
    generate_terrain(x, z, blocks);
    generate_heightmap(blocks, heightmap);
    generate_skylight(blocks, heightmap, skylight);
    
    if (zero_height)
        *zero_height = heightmap[0];
    
    RSTag* level = rs_tag_new(RS_TAG_COMPOUND,
                              "xPos", rs_tag_new(RS_TAG_INT, x),
                              "zPos", rs_tag_new(RS_TAG_INT, z),
                              "Blocks", rs_tag_new(RS_TAG_BYTE_ARRAY, 0x8000, blocks),
                              "BlockLight", rs_tag_new(RS_TAG_BYTE_ARRAY, 0x4000, blocklight),
                              "SkyLight", rs_tag_new(RS_TAG_BYTE_ARRAY, 0x4000, skylight),
                              "Data", rs_tag_new(RS_TAG_BYTE_ARRAY, 0x4000, data),
                              "HeightMap", rs_tag_new(RS_TAG_BYTE_ARRAY, 0x100, heightmap),
                              "Entities", rs_tag_new(RS_TAG_LIST, NULL),
                              "TileEntities", rs_tag_new(RS_TAG_LIST, NULL),
                              "TerrainPopulated", rs_tag_new(RS_TAG_BYTE, 1),
                              "LastUpdate", rs_tag_new(RS_TAG_LONG, 0),
                              NULL);
    
    rs_free(blocks);
    rs_free(blocklight);
    rs_free(skylight);
    rs_free(data);
    rs_free(heightmap);
    
    return rs_tag_new(RS_TAG_COMPOUND, "Level", level, NULL);
}

RSTag* create_level_dat(uint8_t spawn_height)
{
    RSTag* player =
        rs_tag_new(RS_TAG_COMPOUND,
                   /* entity fields */
                   "Pos", rs_tag_new(RS_TAG_LIST,
                                     rs_tag_new(RS_TAG_DOUBLE, 0.0),
                                     rs_tag_new(RS_TAG_DOUBLE, spawn_height + 2.66),
                                     rs_tag_new(RS_TAG_DOUBLE, 0.0),
                                     NULL),
                   "Motion", rs_tag_new(RS_TAG_LIST,
                                        rs_tag_new(RS_TAG_DOUBLE, 0.0),
                                        rs_tag_new(RS_TAG_DOUBLE, 0.0),
                                        rs_tag_new(RS_TAG_DOUBLE, 0.0),
                                        NULL),
                   "Rotation", rs_tag_new(RS_TAG_LIST,
                                          rs_tag_new(RS_TAG_FLOAT, 0.0),
                                          rs_tag_new(RS_TAG_FLOAT, 0.0),
                                          NULL),
                   "FallDistance", rs_tag_new(RS_TAG_FLOAT, 0.0),
                   "Fire", rs_tag_new(RS_TAG_SHORT, -20),
                   "Air", rs_tag_new(RS_TAG_SHORT, 300),
                   "OnGround", rs_tag_new(RS_TAG_BYTE, 1),
                   
                   /* mob fields */
                   "AttackTime", rs_tag_new(RS_TAG_SHORT, 0),
                   "DeathTime", rs_tag_new(RS_TAG_SHORT, 0),
                   "Health", rs_tag_new(RS_TAG_SHORT, 20),
                   "HurtTime", rs_tag_new(RS_TAG_SHORT, 0),
                   
                   /* player fields */
                   "Inventory", rs_tag_new(RS_TAG_LIST, NULL),
                   "Score", rs_tag_new(RS_TAG_INT, 0),
                   "Dimension", rs_tag_new(RS_TAG_INT, 0),
                   NULL);
    
    RSTag* data = rs_tag_new(RS_TAG_COMPOUND,
                             "Time", rs_tag_new(RS_TAG_LONG, 0),
                             "LastPlayed", rs_tag_new(RS_TAG_LONG, 0),
                             "Player", player,
                             "SpawnX", rs_tag_new(RS_TAG_INT, 0),
                             "SpawnY", rs_tag_new(RS_TAG_INT, spawn_height),
                             "SpawnZ", rs_tag_new(RS_TAG_INT, 0),
                             "SizeOnDisk", rs_tag_new(RS_TAG_LONG, 0),
                             "RandomSeed", rs_tag_new(RS_TAG_LONG, 0),
                             "version", rs_tag_new(RS_TAG_INT, 19132),
                             "LevelName", rs_tag_new(RS_TAG_STRING, "mapgen"),
                             "raining", rs_tag_new(RS_TAG_BYTE, 0),
                             "thundering", rs_tag_new(RS_TAG_BYTE, 0),
                             "rainTime", rs_tag_new(RS_TAG_INT, 0),
                             "thunderTime", rs_tag_new(RS_TAG_INT, 0),
                             NULL);
    
    return rs_tag_new(RS_TAG_COMPOUND, "Data", data, NULL);
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "usage: %s [dest]\n", argv[0]);
        return 1;
    }
    
    /* create dest */
    if (mkdir(argv[1], 0777) == -1 && errno != EEXIST)
    {
        fprintf(stderr, "could not create %s\n", argv[1]);
        return 1;
    }
    
    char* tmps = rs_malloc(strlen(argv[1]) + 128);
    
    /* create dest/region */
    sprintf(tmps, "%s/region", argv[1]);
    if (mkdir(tmps, 0777) == -1 && errno != EEXIST)
    {
        fprintf(stderr, "could not create %s\n", tmps);
        rs_free(tmps);
        return 1;
    }
    
    /* iterate through all regions / chunks */
    int radius = 2;
    int i = 0;
    int rx, rz, cx, cz;
    bool success = true;
    uint8_t spawn_height = 64;
    for (rx = -radius; rx < radius; rx++)
    {
        for (rz = -radius; rz < radius; rz++)
        {
            sprintf(tmps, "%s/region/r.%i.%i.mcr", argv[1], rx, rz);
            RSRegion* region = rs_region_open(tmps, true);
            if (!region)
            {
                success = false;
                fprintf(stderr, "could not create %s\n", tmps);
                break;
            }
            
            i++;
            printf("writing region %i of %i ...\n", i, 4 * radius * radius);
            for (cx = 0; cx < 32; cx++)
            {
                for (cz = 0; cz < 32; cz++)
                {
                    RSTag* chunk = NULL;
                    if (rx == 0 && rz == 0 && cx == 0 && cz == 0)
                    {
                        chunk = create_chunk(rx * 32 + cx, rz * 32 + cz, &spawn_height);
                    } else {
                        chunk = create_chunk(rx * 32 + cx, rz * 32 + cz, NULL);
                    }
                    RSNBT* nbt = rs_nbt_new();
                    rs_nbt_set_root(nbt, chunk);
                    success = rs_nbt_write_to_region(nbt, region, cx, cz);
                    rs_nbt_free(nbt);
                    
                    if (!success)
                    {
                        fprintf(stderr, "error generating chunks\n");
                        break;
                    }
                }
                
                if (!success)
                    break;
            }
            
            rs_region_close(region);
            
            if (!success)
                break;
        }
        
        if (!success)
            break;
    }
    
    if (!success)
    {
        rs_free(tmps);
        return 1;
    }

    /* create level.dat file */
    sprintf(tmps, "%s/level.dat", argv[1]);
    RSTag* level_dat = create_level_dat(spawn_height);
    RSNBT* level_nbt = rs_nbt_new();
    rs_nbt_set_root(level_nbt, level_dat);
    success = rs_nbt_write_to_file(level_nbt, tmps);
    rs_nbt_free(level_nbt);
    
    rs_free(tmps);
    
    if (!success)
    {
        fprintf(stderr, "error writing level.dat\n");
        return 1;
    }
    
    return 0;
}
