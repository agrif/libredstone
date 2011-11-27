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

#include "config.h"
#include "options.h"
#include "actions.h"
#include "formats.h"
#include "optparse.h"

RSToolOptions* popts = NULL;

static int opt_set_action(char* val, void* data)
{
    rs_assert(!val);
    popts->action = (size_t)data;
    return 0;
}

int main(int argc, char* argv[])
{
    RSToolOptions opts = {
        .action = RS_EXTRACT,
    };
    popts = &opts;
    
    const char** formatters = rs_tool_formatter_names();
    
    struct opt_spec option_list[] = {
        {opt_help, "h", "--help", OPT_NO_METAVAR, OPT_NO_HELP, OPT_NO_DATA},
        {opt_version, "v", "--version", OPT_NO_METAVAR, OPT_NO_HELP, PACKAGE " " LIBREDSTONE_VERSION},
        {opt_set_action, "e", "--extract", OPT_NO_METAVAR, "extract the given file (most useful for regions)", (void*)RS_EXTRACT},
        {opt_store_choice, "f", "--format", "FORMAT", "use the given format for -e (see --list-formats)", formatters},
        {rs_tool_list_formatters, OPT_NO_SF, "--list-formats", OPT_NO_METAVAR, "list the available formats", OPT_NO_DATA},
        {OPT_NO_ACTION}
    };
    
    opt_basename(argv[0], '/');
    int args = opt_parse("usage: %s [options] { level.dat | region.mcr x z }", option_list, argv);
    
    opts.formatter = rs_tool_get_formatter(formatters[0]);
    rs_assert(opts.formatter);
    
    /* read in the source positional arguments */
    if (!(args == 1 || args == 3))
    {
        fprintf(stderr, "%s: no valid file given\n", argv[0]);
        fprintf(stderr, "%s: see --help for details\n", argv[0]);
        return 1;
    } else if (args == 1) {
        opts.source.type = RS_STANDALONE;
        for (int i = 1; i < argc; i++)
        {
            if (argv[i][0] != 0)
            {
                opts.source.standalone = argv[i];
                break;
            }
        }
    } else if (args == 3) {
        opts.source.type = RS_REGION;
        for (int i = 1; i < argc; i++)
        {
            if (argv[i][0] != 0)
            {
                char* end;
                long int tmp;
                switch (args)
                {
                case 3:
                    opts.source.region.path = argv[i];
                    break;
                case 2:
                    tmp = strtol(argv[i], &end, 10);
                    if (end[0] != 0 || tmp < 0 || tmp >= 32)
                    {
                        fprintf(stderr, "%s: invalid integer for x: %s\n", argv[0], argv[i]);
                        fprintf(stderr, "%s: see --help for details\n", argv[0]);
                        return 1;
                    }
                    opts.source.region.x = tmp;
                    break;
                case 1:
                    tmp = strtol(argv[i], &end, 10);
                    if (end[0] != 0 || tmp < 0 || tmp >= 32)
                    {
                        fprintf(stderr, "%s: invalid integer for z: %s\n", argv[0], argv[i]);
                        fprintf(stderr, "%s: see --help for details\n", argv[0]);
                        return 1;
                    }
                    opts.source.region.z = tmp;
                    break;
                };
                
                args--;
            }
        }
        
        rs_assert(args == 0);
    }
    
    /* open the source */
    switch (opts.source.type)
    {
    case RS_REGION:
        opts.source.region.region = rs_region_open(opts.source.region.path, false);
        if (!opts.source.region.region)
        {
            fprintf(stderr, "%s: could not open region: %s\n", argv[0], opts.source.region.path);
            return 1;
        }
        
        opts.source.nbt = rs_nbt_parse_from_region(opts.source.region.region, opts.source.region.x, opts.source.region.z);
        if (!opts.source.nbt)
        {
            fprintf(stderr, "%s: could not open chunk (%i, %i) in region: %s\n", argv[0], opts.source.region.x, opts.source.region.z, opts.source.region.path);
            return 1;
        }
        
        break;
    case RS_STANDALONE:
        opts.source.nbt = rs_nbt_parse_from_file(opts.source.standalone);
        if (!opts.source.nbt)
        {
            fprintf(stderr, "%s: could not open NBT file: %s\n", argv[0], opts.source.standalone);
            return 1;
        }
        break;
    };
    
    /* do things */
    int ret = 0;
    switch (opts.action)
    {
    case RS_EXTRACT:
        ret = rs_tool_extract(&opts);
        break;
    };
    
    /* close the source */
    if (opts.source.type == RS_REGION)
    {
        rs_region_close(opts.source.region.region);
    }
    rs_nbt_free(opts.source.nbt);
    
    rs_free(formatters);
    
    return ret;
}
