/** @file
 * @brief Acknowledge only a completely written and flushed domain response.
 * [PROJECT-01](../../../../spec/implementation/project-tools.md#project-01)
 * separates compiler analysis from project mutation;
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * preserves original bindings, capture state and qualified constant members;
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) retains exact
 * bytes and source coordinates and refuses incomplete output as success.
 */
#include "internal.h"
bool delivery_source_report(FILE *output, const ModuleCatalog *catalog,
    const DeliverySourceDefinition *definitions) {
    if (!output || !catalog || !catalog->built) { return false; }
    ToolJsonWriter writer;
    tool_json_init(&writer, output);
    return tool_json_raw(&writer, "{\"format\":\"ama.source-domain.v1\",\"inputsSealed\":false,")
        && delivery_source_report_catalog(&writer, catalog)
        && tool_json_raw(&writer, ",")
        && delivery_source_report_graph(&writer, definitions)
        && tool_json_raw(&writer, "}\n")
        && tool_json_finish(&writer);
}
