/** @file
 * @brief Stage the selected delivery and exact launch-input capability manifest.
 * [CLI-01](../../../../spec/implementation/launch-and-embedding.md#cli-01)
 * keeps runtime arguments outside compilation configuration;
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) requires a complete
 * owned artifact before any publication. Native/build provenance is added by T239.
 */
#include <string.h>
#include "internal.h"
/** Emit every declared input, including omitted/default-only inputs. A null binary
 * adapter never claims a decoder under
 * [CLI-03](../../../../spec/implementation/launch-and-embedding.md#cli-03).
 */
static bool inputs(ToolJsonWriter *writer, const NativeScaffold *scaffold) {
    if (!tool_json_raw(writer, "[")) { return false; }
    for (size_t index = 0; index < scaffold->launch_inputs->count; index++) {
        const DeliveryInput *input = &scaffold->launch_inputs->entries[index];
        SourceSpan name = input->field->name;
        if ((index && !tool_json_raw(writer, ",")) || !tool_json_raw(writer, "{\"name\":")
            || !tool_json_text(writer, source_bytes(name.source) + name.start, name.end - name.start)
            || !tool_json_raw(writer, ",\"ordinal\":") || !tool_json_unsigned(writer, input->ordinal)
            || !tool_json_raw(writer, ",\"text\":") || !tool_json_boolean(writer, input->text_supported)
            || !tool_json_raw(writer, ",\"binary\":null}")) { return false; }
    }
    return tool_json_raw(writer, "]");
}
bool codegen_emit_delivery(NativeScaffold *scaffold, FILE *output, const ScaffoldType *unused) {
    (void)unused;
    ToolJsonWriter writer;
    tool_json_init(&writer, output);
    return tool_json_raw(&writer, "{\"format\":\"ama.delivery.v1\",\"mode\":")
        && tool_json_string(&writer, scaffold->delivery->mode == DELIVERY_EXECUTABLE ? "executable" : "source-inclusion")
        && tool_json_raw(&writer, ",\"component\":") && tool_json_string(&writer, scaffold->delivery->component)
        && tool_json_raw(&writer, ",\"root\":") && tool_json_unsigned(&writer, codegen_type_id(scaffold, scaffold->selected))
        && tool_json_raw(&writer, ",\"directory\":")
        && delivery_metadata_bytes(&writer, (const unsigned char *)scaffold->directory, strlen(scaffold->directory))
        && tool_json_raw(&writer, ",\"types\":") && delivery_metadata_types(&writer, scaffold)
        && tool_json_raw(&writer, ",\"inputs\":") && inputs(&writer, scaffold)
        && delivery_metadata_outputs(&writer, scaffold)
        && tool_json_raw(&writer, "}\n") && tool_json_finish(&writer);
}
