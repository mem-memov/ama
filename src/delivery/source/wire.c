/** @file
 * @brief Encode nullable physical paths and captured source bytes losslessly.
 * [PROJECT-01](../../../spec/implementation/project-tools.md#project-01)
 * separates compiler analysis from project mutation;
 * [TOOL-02](../../../spec/implementation/tools-and-conformance.md#tool-02)
 * preserves original bindings, capture state and qualified constant members;
 * [COMP-04](../../../spec/implementation/compiler.md#comp-04) retains exact
 * bytes and source coordinates and refuses incomplete output as success.
 */
#include <string.h>
#include "wire.h"
bool delivery_source_wire_path(ToolJsonWriter *writer, const char *path) {
    return path ? tool_json_bytes(writer, (const unsigned char *)path, strlen(path))
        : tool_json_raw(writer, "null");
}
bool delivery_source_wire_text(ToolJsonWriter *writer, const SourceText *text) {
    if (!text) { return tool_json_raw(writer, "null"); }
    return tool_json_raw(writer, "{\"path\":")
        && delivery_source_wire_path(writer, source_path(text))
        && tool_json_raw(writer, ",\"content\":")
        && tool_json_bytes(writer, source_bytes(text), source_length(text))
        && tool_json_raw(writer, "}");
}
