/** @file
 * @brief Preserve arbitrary host-path and framed-identity bytes in delivery JSON.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) requires exact
 * metadata: invalid UTF-8 host bytes cannot be normalized or discarded.
 */
#include "internal.h"
bool delivery_metadata_bytes(ToolJsonWriter *writer, const unsigned char *bytes, size_t length) {
    const char digits[] = "0123456789abcdef";
    if (!tool_json_raw(writer, "{\"hex\":\"")) { return false; }
    for (size_t index = 0; index < length; index++) {
        char pair[] = {digits[bytes[index] >> 4], digits[bytes[index] & 15], 0};
        if (!tool_json_raw(writer, pair)) { return false; }
    }
    return tool_json_raw(writer, "\",\"length\":") && tool_json_unsigned(writer, length)
        && tool_json_raw(writer, "}");
}
