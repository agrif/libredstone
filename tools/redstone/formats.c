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

#include "formats.h"
#include "config.h"
#include <string.h>

extern RSToolFormatter rs_tool_formatter_prettyprint;
extern RSToolFormatter rs_tool_formatter_raw;
#ifdef ENABLE_LIBXML
extern RSToolFormatter rs_tool_formatter_xml;
#endif

RSToolFormatter* rs_tool_formatters[] = {
    &rs_tool_formatter_prettyprint,
    &rs_tool_formatter_raw,
#ifdef ENABLE_LIBXML
    &rs_tool_formatter_xml,
#endif
    NULL
};

const char** rs_tool_formatter_names(void)
{
    unsigned int size = 0;
    while (rs_tool_formatters[size] != NULL)
        size++;
    
    const char** names = rs_new0(const char*, size + 1);
    
    for (unsigned int i = 0; i < size; i++)
    {
        names[i] = rs_tool_formatters[i]->name;
    }
    return names;
}

RSToolFormatter* rs_tool_get_formatter(const char* name)
{
    for (unsigned int i = 0;  rs_tool_formatters[i] != NULL; i++)
    {
        if (strcmp(name, rs_tool_formatters[i]->name) == 0)
            return rs_tool_formatters[i];
    }
    return NULL;
}

int rs_tool_list_formatters(char* val, void* data)
{
    rs_assert(val == NULL);
    
    unsigned int indent_size = 0;
    for (unsigned int i = 0;  rs_tool_formatters[i] != NULL; i++)
    {
        unsigned int len = strlen(rs_tool_formatters[i]->name);
        if (len > indent_size)
            indent_size = len;
    }
    
    for (unsigned int i = 0;  rs_tool_formatters[i] != NULL; i++)
    {
        printf(" %*s -- %s\n", indent_size, rs_tool_formatters[i]->name, rs_tool_formatters[i]->description);
    }
    
    exit(0);
}
