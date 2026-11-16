/** @file
 * @brief Serialize exact numeric identities and arbitrary byte strings without floating-point conversion.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04)
 * preserves source coordinates and physical names independently of JSON consumer numeric/Unicode limits.
 */
#include <inttypes.h>
#include "internal.h"
#include "source/unicode/unicode.h"
/** The maximum decimal buffer follows the integer type width; checked formatting cannot silently truncate an identity. Component contract: [COMP-04](../../../../spec/implementation/compiler.md#comp-04). */
bool tool_json_unsigned(ToolJsonWriter *writer, uintmax_t value) {
    char text[3*sizeof(value)+3];
    int length = snprintf(text,sizeof(text),"%ju",value);
    if (length < 0 || (size_t)length >= sizeof(text)) {
        if (writer->status == TOOL_JSON_OK) { writer->status = TOOL_JSON_IO_ERROR; }
        return false;
    }
    return tool_json_text(writer,(const unsigned char *)text,(size_t)length);
}
/** Wire booleans have exactly the standard lowercase JSON spellings. Component contract: [COMP-04](../../../../spec/implementation/compiler.md#comp-04). */
bool tool_json_boolean(ToolJsonWriter *writer, bool value) { return tool_json_raw(writer,value ? "true" : "false"); }
/** A displayable UTF-8 view is optional; hexadecimal bytes always preserve the exact physical name or payload. Component contract: [COMP-04](../../../../spec/implementation/compiler.md#comp-04). */
bool tool_json_bytes(ToolJsonWriter *writer, const unsigned char *bytes, size_t length) {
    if (writer->status != TOOL_JSON_OK) { return false; }
    if (bytes == NULL && length != 0) { writer->status = TOOL_JSON_INVALID_UTF8; return false; }
    size_t error;
    bool valid = utf8_validate(bytes,length,&error);
    if (!tool_json_raw(writer,"{\"utf8\":")) { return false; }
    if (!(valid ? tool_json_text(writer,bytes,length) : tool_json_raw(writer,"null"))
        || !tool_json_raw(writer,",\"hex\":\"")) { return false; }
    const char digits[] = "0123456789abcdef";
    for (size_t index = 0; index < length; index++) {
        char pair[] = {digits[bytes[index]>>4],digits[bytes[index]&15]};
        if (!tool_json_write(writer,pair,sizeof(pair))) { return false; }
    }
    return tool_json_raw(writer,"\"}");
}
