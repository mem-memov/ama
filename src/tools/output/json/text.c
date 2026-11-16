/** @file
 * @brief Escape exact UTF-8/control bytes without trimming, normalization or lossy substitution.
 * [SRC-03](../../../../spec/language/source-and-names.md#src-03)
 * keeps original source bytes and line endings observable through tool responses.
 */
#include <string.h>
#include "internal.h"
#include "source/unicode/unicode.h"
/** Validate before opening the quote; control scalars use exact ASCII escapes and all other UTF-8 bytes stay unchanged. Component contract: [SRC-03](../../../../spec/language/source-and-names.md#src-03). */
bool tool_json_text(ToolJsonWriter *writer, const unsigned char *bytes, size_t length) {
    if (writer->status != TOOL_JSON_OK) { return false; }
    size_t error;
    if (!utf8_validate(bytes,length,&error)) { writer->status = TOOL_JSON_INVALID_UTF8; return false; }
    if (!tool_json_raw(writer,"\"")) { return false; }
    const char digits[] = "0123456789abcdef";
    size_t start = 0;
    for (size_t offset = 0; offset < length; offset++) {
        unsigned char value = bytes[offset];
        if (value >= 32 && value != '"' && value != '\\') { continue; }
        if (!tool_json_write(writer,bytes+start,offset-start)) { return false; }
        if (value < 32) {
            char escaped[] = {'\\','u','0','0',digits[value>>4],digits[value&15]};
            if (!tool_json_write(writer,escaped,sizeof(escaped))) { return false; }
        } else {
            char escaped[] = {'\\',(char)value};
            if (!tool_json_write(writer,escaped,sizeof(escaped))) { return false; }
        }
        start = offset+1;
    }
    return tool_json_write(writer,length == 0 ? NULL : bytes+start,length-start) && tool_json_raw(writer,"\"");
}
/** C strings remain nullable; byte-view encoding is used for source text with embedded zeros. Component contract: [SRC-03](../../../../spec/language/source-and-names.md#src-03). */
bool tool_json_string(ToolJsonWriter *writer, const char *text) {
    return text == NULL ? tool_json_raw(writer,"null") : tool_json_text(writer,(const unsigned char *)text,strlen(text));
}
