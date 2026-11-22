/** @file
 * @brief Serialize authoritative version, module and native capture metadata.
 * [PROJECT-01](../../../../spec/implementation/project-tools.md#project-01)
 * separates compiler analysis from project mutation;
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * preserves original bindings, capture state and qualified constant members;
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) retains exact
 * bytes and source coordinates and refuses incomplete output as success.
 */
#include "internal.h"
/** Preserve configuration/native capture state without opening companion paths.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * distinguishes authoritative observed absence from uncaptured inputs.
 */
static bool source(ToolJsonWriter *writer, const ModuleSource *item) {
    return tool_json_raw(writer, "{\"identity\":")
        && tool_json_string(writer, item->logical_path)
        && tool_json_raw(writer, ",\"source\":")
        && delivery_source_wire_text(writer, item->text)
        && tool_json_raw(writer, ",\"configurationCaptured\":")
        && tool_json_boolean(writer, item->configuration_loaded)
        && tool_json_raw(writer, ",\"configuration\":")
        && delivery_source_wire_text(writer, item->configuration)
        && tool_json_raw(writer, ",\"native\":{\"captured\":")
        && tool_json_boolean(writer, item->native_loaded)
        && tool_json_raw(writer, ",\"body\":")
        && delivery_source_wire_path(writer, item->native_c)
        && tool_json_raw(writer, ",\"header\":")
        && delivery_source_wire_path(writer, item->native_header)
        && tool_json_raw(writer, ",\"abi\":")
        && delivery_source_wire_text(writer, item->native_abi)
        && tool_json_raw(writer, "}}");
}
bool delivery_source_report_catalog(ToolJsonWriter *writer, const ModuleCatalog *catalog) {
    if (!tool_json_raw(writer, "\"versions\":[")) { return false; }
    bool comma = false;
    for (const ModuleVersion *item = catalog->versions; item; item = item->next) {
        if ((comma && !tool_json_raw(writer, ","))
            || !tool_json_raw(writer, "{\"identity\":")
            || !tool_json_string(writer, item->logical_path)
            || !tool_json_raw(writer, ",\"directory\":")
            || !delivery_source_wire_path(writer, item->physical_path)
            || !tool_json_raw(writer, ",\"configuration\":")
            || !delivery_source_wire_text(writer, item->configuration)
            || !tool_json_raw(writer, "}")) { return false; }
        comma = true;
    }
    if (!tool_json_raw(writer, "],\"sources\":[")) { return false; }
    comma = false;
    for (const ModuleSource *item = catalog->first; item; item = item->next) {
        if ((comma && !tool_json_raw(writer, ",")) || !source(writer, item)) { return false; }
        comma = true;
    }
    return tool_json_raw(writer, "]");
}
