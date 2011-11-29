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

#ifndef __RS_TOOL_OPTIONS_H_INCLUDED__
#define __RS_TOOL_OPTIONS_H_INCLUDED__

#include "redstone.h"
#include "formats.h"

typedef struct
{
    char** argv;
    int argc;
    
    struct
    {
        enum
        {
            RS_STANDALONE,
            RS_REGION,
        } type;
        union
        {
            const char* standalone;
            struct
            {
                const char* path;
                uint8_t x, z;
                RSRegion* region;
            } region;
        };
        RSNBT* nbt;
    } source;
    
    enum
    {
        RS_EXTRACT,
    } action;
    
    void (*error)(const char* fmt, ...);
    RSToolFormatter* formatter;
} RSToolOptions;

#endif /* __RS_TOOL_OPTIONS_H_INCLUDED__ */
