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

#ifndef __RS_TOOL_FORMATS_H_INCLUDED__
#define __RS_TOOL_FORMATS_H_INCLUDED__

#include "redstone.h"
#include <stdio.h>

typedef struct
{
    const char* name;
    const char* description;
    void (*dump)(RSNBT*, FILE*);
} RSToolFormatter;

extern RSToolFormatter* rs_tool_formatter[];

const char** rs_tool_formatter_names(void);
RSToolFormatter* rs_tool_get_formatter(const char* name);
int rs_tool_list_formatters(char* val, void* data);

#endif /* __RS_TOOL_FORMATS_H_INCLUDED__ */
