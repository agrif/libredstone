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

static void prettyprint_dump(RSToolOptions* opts, RSNBT* nbt, FILE* out)
{
    rs_nbt_pretty_print(nbt, out);
}

RSToolFormatter rs_tool_formatter_prettyprint = {
    .name = "prettyprint",
    .description = "a pretty (but non-standard) representation",
    .dump = prettyprint_dump,
};
