/** @file
 * @brief Serialize original definition categories, dependency targets and source sites.
 * [PROJECT-01](../../../../spec/implementation/project-tools.md#project-01)
 * separates compiler analysis from project mutation;
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * preserves original bindings, capture state and qualified constant members;
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) retains exact
 * bytes and source coordinates and refuses incomplete output as success.
 */
#include "internal.h"
/** A requiring construct is not necessarily a written path. Preserve its source
 * occurrence alongside the compiler's normalized target/member instead of
 * splitting source text under
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02).
 */
static bool edge(ToolJsonWriter *writer, const DeliverySourceDependency *item) {
    return tool_json_raw(writer, "{\"target\":")
        && tool_json_string(writer, item->target)
        && tool_json_raw(writer, ",\"implicit\":")
        && tool_json_boolean(writer, item->implicit)
        && tool_json_raw(writer, ",\"member\":")
        && tool_json_string(writer, item->member)
        && tool_json_raw(writer, ",\"origin\":{\"path\":")
        && delivery_source_report_path(writer, item->origin.source ? source_path(item->origin.source) : NULL)
        && tool_json_raw(writer, ",\"start\":")
        && tool_json_unsigned(writer, item->origin.start)
        && tool_json_raw(writer, ",\"end\":")
        && tool_json_unsigned(writer, item->origin.end)
        && tool_json_raw(writer, "}}");
}
bool delivery_source_report_graph(ToolJsonWriter *writer, const DeliverySourceDefinition *definitions) {
    static const char *const kinds[] = {"concrete", "parameterized", "static", "constants"};
    if (!tool_json_raw(writer, "\"definitions\":[")) { return false; }
    bool comma = false;
    for (const DeliverySourceDefinition *item = definitions; item; item = item->next) {
        if ((unsigned)item->kind >= sizeof(kinds) / sizeof(*kinds)) { return false; }
        if ((comma && !tool_json_raw(writer, ","))
            || !tool_json_raw(writer, "{\"identity\":")
            || !tool_json_string(writer, item->identity)
            || !tool_json_raw(writer, ",\"kind\":")
            || !tool_json_string(writer, kinds[item->kind])
            || !tool_json_raw(writer, ",\"dependencies\":[")) { return false; }
        bool separator = false;
        for (const DeliverySourceDependency *dependency = item->dependencies; dependency; dependency = dependency->next) {
            if ((separator && !tool_json_raw(writer, ",")) || !edge(writer, dependency)) { return false; }
            separator = true;
        }
        if (!tool_json_raw(writer, "]}")) { return false; }
        comma = true;
    }
    return tool_json_raw(writer, "]");
}
