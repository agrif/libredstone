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
#include <stdarg.h>

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

static void error(const char* fmt, ...)
{
    va_list arg;
    va_start(arg, fmt);
    fprintf(stderr, "%s: ", popts->argv[0]);
    vfprintf(stderr, fmt, arg);
    va_end(arg);
}

bool standalone_save_nbt(RSNBT* newnbt)
{
    rs_assert(popts->source.type == RS_STANDALONE);
    rs_assert(newnbt);
    if(!rs_nbt_write_to_file(newnbt, popts->source.standalone))
    {
        popts->error("could not write to file `%s'\n", popts->source.standalone);
        return false;
    }
    return true;
}

bool region_save_nbt(RSNBT* newnbt)
{
    rs_assert(popts->source.type == RS_REGION);
    rs_assert(newnbt);
    if (!rs_nbt_write_to_region(newnbt, popts->source.region.region, popts->source.region.x, popts->source.region.z))
    {
        popts->error("could not write to chunk (%i, %i) in region `%s'\n", popts->source.region.x, popts->source.region.z, popts->source.region.path);
        return false;
    }
    rs_region_flush(popts->source.region.region);
    return true;
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
        {opt_text, OPT_NO_SF, OPT_NO_LF, OPT_NO_METAVAR, " ", OPT_NO_DATA},
        
        {opt_text, OPT_NO_SF, OPT_NO_LF, OPT_NO_METAVAR, "\nActions:", OPT_NO_DATA},
        {opt_set_action, "e", "--extract", OPT_NO_METAVAR, "extract the given file to stdout (most useful for regions)", (void*)RS_EXTRACT},
        {opt_set_action, "r", "--replace", OPT_NO_METAVAR, "replace the given file with stdin (most useful for regions)", (void*)RS_REPLACE},
        
        {opt_text, OPT_NO_SF, OPT_NO_LF, OPT_NO_METAVAR, "\nOptions for extract, replace:", OPT_NO_DATA},
        {opt_store_choice, "f", "--format", "FORMAT", "use the given format for input/output", formatters},
        {rs_tool_list_formatters, OPT_NO_SF, "--list-formats", OPT_NO_METAVAR, "list the available formats", OPT_NO_DATA},
        {OPT_NO_ACTION}
    };
    
    opt_basename(argv[0], '/');
    int args = opt_parse("usage: %s [options] { level.dat | region.mcr x z }", option_list, argv);
    
    opts.argv = argv;
    opts.argc = argc;
    opts.error = error;
    
    opts.formatter = rs_tool_get_formatter(formatters[0]);
    rs_assert(opts.formatter);
    
    /* read in the source positional arguments */
    if (!(args == 1 || args == 3))
    {
        error("no valid file given\n");
        error("see --help for details\n");
        return 1;
    } else if (args == 1) {
        opts.source.type = RS_STANDALONE;
        opts.save_nbt = standalone_save_nbt;
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
        opts.save_nbt = region_save_nbt;
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
                        error("invalid integer for x: %s\n", argv[i]);
                        error("see --help for details\n");
                        return 1;
                    }
                    opts.source.region.x = tmp;
                    break;
                case 1:
                    tmp = strtol(argv[i], &end, 10);
                    if (end[0] != 0 || tmp < 0 || tmp >= 32)
                    {
                        error("invalid integer for z: %s\n", argv[i]);
                        error("see --help for details\n");
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
    bool write = false;
    if (opts.action == RS_REPLACE)
        write = true;
    switch (opts.source.type)
    {
    case RS_REGION:
        opts.source.region.region = rs_region_open(opts.source.region.path, write);
        if (!opts.source.region.region)
        {
            error("could not open region: %s\n", opts.source.region.path);
            return 1;
        }
        
        opts.source.nbt = rs_nbt_parse_from_region(opts.source.region.region, opts.source.region.x, opts.source.region.z);
        if (!write && !opts.source.nbt)
        {
            error("could not open chunk (%i, %i) in region: %s\n", opts.source.region.x, opts.source.region.z, opts.source.region.path);
            return 1;
        }
        
        break;
    case RS_STANDALONE:
        opts.source.nbt = rs_nbt_parse_from_file(opts.source.standalone);
        if (!write && !opts.source.nbt)
        {
            error("could not open NBT file: %s\n", opts.source.standalone);
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
    case RS_REPLACE:
        ret = rs_tool_replace(&opts);
        break;
    };
    
    /* close the source */
    if (opts.source.type == RS_REGION)
    {
        rs_region_close(opts.source.region.region);
    }
    if (opts.source.nbt)
        rs_nbt_free(opts.source.nbt);
    
    rs_free(formatters);
    
    return ret;
}
