/** @file
 * @brief Record actual prepared outputs and the explicitly selected verification unit.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) gives complete
 * owned artifact bytes; [STACK-04](../../../../spec/implementation/compiler.md#stack-04)
 * requires exact unit selection instead of a filename-based entry guess.
 */
#include <string.h>
#include "internal.h"
/** Preserve physical path bytes without normalization or shell interpolation.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) retains exact
 * recorded destinations through publication.
 */
static bool path(ToolJsonWriter *writer, const char *value) {
    return delivery_metadata_bytes(writer, (const unsigned char *)value, strlen(value));
}
bool delivery_metadata_outputs(ToolJsonWriter *writer, const NativeScaffold *scaffold) {
    if (!scaffold->entry_artifact || !tool_json_raw(writer, ",\"generated\":[")) { return false; }
    bool first = true;
    for (const ScaffoldArtifact *artifact = scaffold->first; artifact; artifact = artifact->next) {
        if ((!first && !tool_json_raw(writer, ",")) || !tool_json_raw(writer, "{\"path\":")
            || !path(writer, artifact->path) || !tool_json_raw(writer, ",\"length\":")
            || !tool_json_unsigned(writer, source_length(artifact->content))
            || !tool_json_raw(writer, ",\"entry\":")
            || !tool_json_boolean(writer, artifact == scaffold->entry_artifact)
            || !tool_json_raw(writer, "}")) { return false; }
        first = false;
    }
    return tool_json_raw(writer, "],\"verification_entries\":[{\"path\":")
        && path(writer, scaffold->entry_artifact->path) && tool_json_raw(writer, ",\"main\":")
        && tool_json_boolean(writer, scaffold->delivery->mode == DELIVERY_EXECUTABLE)
        && tool_json_raw(writer, "}]");
}
