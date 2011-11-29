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
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>

static inline void xml_start_tag(xmlTextWriterPtr writer, const char* type, const char* name)
{
    int rc;
    rc = xmlTextWriterStartElement(writer, BAD_CAST type);
    rs_return_if_fail(rc >= 0);
    if (name && strlen(name) > 0)
    {
        rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "name", BAD_CAST name);
        rs_return_if_fail(rc >= 0);
    }
}

static void xml_dump_tag(RSTag* tag, xmlTextWriterPtr writer, const char* name)
{
    RSTagIterator it;
    RSTag* subtag;
    const char* subname;
    int rc;

    switch (rs_tag_get_type(tag))
    {
    case RS_TAG_BYTE:
        xml_start_tag(writer, "byte", name);
        rc = xmlTextWriterWriteFormatString(writer, "%i", (int)rs_tag_get_integer(tag));
        rs_return_if_fail(rc >= 0);
        break;
    case RS_TAG_SHORT:
        xml_start_tag(writer, "short", name);
        rc = xmlTextWriterWriteFormatString(writer, "%i", (int)rs_tag_get_integer(tag));
        rs_return_if_fail(rc >= 0);
        break;
    case RS_TAG_INT:
        xml_start_tag(writer, "int", name);
        rc = xmlTextWriterWriteFormatString(writer, "%i", (int)rs_tag_get_integer(tag));
        rs_return_if_fail(rc >= 0);
        break;
    case RS_TAG_LONG:
        xml_start_tag(writer, "long", name);
        rc = xmlTextWriterWriteFormatString(writer, "%li", (long)rs_tag_get_integer(tag));
        rs_return_if_fail(rc >= 0);
        break;
    case RS_TAG_FLOAT:
        xml_start_tag(writer, "float", name);
        rc = xmlTextWriterWriteFormatString(writer, "%f", rs_tag_get_float(tag));
        rs_return_if_fail(rc >= 0);
        break;
    case RS_TAG_DOUBLE:
        xml_start_tag(writer, "double", name);
        rc = xmlTextWriterWriteFormatString(writer, "%f", rs_tag_get_float(tag));
        rs_return_if_fail(rc >= 0);
        break;
    case RS_TAG_BYTE_ARRAY:
        xml_start_tag(writer, "bytearray", name);
        rc = xmlTextWriterWriteString(writer, BAD_CAST "\n");
        rs_return_if_fail(rc >= 0);
        rc = xmlTextWriterWriteBase64(writer, (const char*)rs_tag_get_byte_array(tag), 0, rs_tag_get_byte_array_length(tag));
        rs_return_if_fail(rc >= 0);
        rc = xmlTextWriterWriteString(writer, BAD_CAST "\n");
        rs_return_if_fail(rc >= 0);
        break;
    case RS_TAG_STRING:
        xml_start_tag(writer, "string", name);
        rc = xmlTextWriterWriteString(writer, BAD_CAST rs_tag_get_string(tag));
        rs_return_if_fail(rc >= 0);
        break;
    case RS_TAG_LIST:
        xml_start_tag(writer, "list", name);
        rs_tag_list_iterator_init(tag, &it);
        while (rs_tag_list_iterator_next(&it, &subtag))
        {
            xml_dump_tag(subtag, writer, NULL);
        }
        break;
    case RS_TAG_COMPOUND:
        xml_start_tag(writer, "compound", name);
        rs_tag_compound_iterator_init(tag, &it);
        while (rs_tag_compound_iterator_next(&it, &subname, &subtag))
        {
            xml_dump_tag(subtag, writer, subname);
        }
        break;
    default:
        /* unhandled case !!! */
        rs_critical("got type %i", rs_tag_get_type(tag));
        rs_return_if_reached();
    }
    
    rc = xmlTextWriterEndElement(writer);
    rs_return_if_fail(rc >= 0);
}

static void xml_dump(RSToolOptions* opts, RSNBT* nbt, FILE* out)
{
    LIBXML_TEST_VERSION;
    
    xmlTextWriterPtr writer;
    xmlDocPtr doc;
    int rc;
    
    writer = xmlNewTextWriterDoc(&doc, 0);
    rs_return_if_fail(writer);
    
    rc = xmlTextWriterStartDocument(writer, NULL, "UTF-8", NULL);
    rs_return_if_fail(rc >= 0);
    
    rc = xmlTextWriterWriteDTD(writer, BAD_CAST "nbt", BAD_CAST "+//IDN libredstone.org//DTD XML-NBT//EN", BAD_CAST "http://libredstone.org/dtd/nbt.dtd", BAD_CAST "");
    rs_return_if_fail(rc >= 0);
    
    rc = xmlTextWriterStartElement(writer, BAD_CAST "nbt");
    rs_return_if_fail(rc >= 0);
    const char* name = rs_nbt_get_name(nbt);
    if (name != NULL && strlen(name) > 0)
    {
        rc = xmlTextWriterWriteAttribute(writer, BAD_CAST "name", BAD_CAST rs_nbt_get_name(nbt));
        rs_return_if_fail(rc >= 0);
    }
    
    xml_dump_tag(rs_nbt_get_root(nbt), writer, NULL);
    
    rc = xmlTextWriterEndElement(writer);
    rs_return_if_fail(rc >= 0);
    
    rc = xmlTextWriterEndDocument(writer);
    rs_return_if_fail(rc >= 0);
    
    xmlFreeTextWriter(writer);
    xmlDocFormatDump(out, doc, 1);
    xmlFreeDoc(doc);
}

RSToolFormatter rs_tool_formatter_xml = {
    .name = "xml",
    .description = "an xml representation of NBT",
    .dump = xml_dump,
};
