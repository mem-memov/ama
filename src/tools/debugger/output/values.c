/** @file
 * @brief Preserve exact debugger identities, statuses and original source locations in JSON.
 * [REF-02](../../../../spec/language/values-and-references.md#ref-02) keeps generation
 * identities exact; [DEBUG-01](../../../../spec/implementation/diagnostics.md#debug-01) keeps source coordinates separate from hashes.
 */
#include <string.h>
#include "../internal.h"
/** Message bytes need no terminating NUL and are never parsed to classify a status. */
bool ama_debug_client_status(ToolJsonWriter *writer, AmaStatus status) {
    return tool_json_raw(writer, "{\"code\":") && tool_json_unsigned(writer, status.code)
        && tool_json_raw(writer, ",\"message\":")
        && tool_json_bytes(writer, (const unsigned char *)status.message, status.message_length)
        && tool_json_raw(writer, "}");
}
/** A context-qualified numeric observation grants no future access without revalidation. */
bool ama_debug_client_reference(ToolJsonWriter *writer, AmaContextReference reference) {
    return tool_json_raw(writer, "{\"context\":") && tool_json_unsigned(writer, reference.context)
        && tool_json_raw(writer, ",\"type\":") && tool_json_unsigned(writer, reference.local.type_id)
        && tool_json_raw(writer, ",\"slot\":") && tool_json_unsigned(writer, reference.local.slot_id)
        && tool_json_raw(writer, ",\"generation\":") && tool_json_unsigned(writer, reference.local.generation)
        && tool_json_raw(writer, "}");
}
/** Occurrence identity remains distinct even for repeated or absent target values. */
bool ama_debug_client_membership(ToolJsonWriter *writer, uint64_t context, AmaMembership membership) {
    return tool_json_raw(writer, "{\"owner\":")
        && ama_debug_client_reference(writer, (AmaContextReference){context, membership.owner})
        && tool_json_raw(writer, ",\"field\":") && tool_json_unsigned(writer, membership.field)
        && tool_json_raw(writer, ",\"incarnation\":") && tool_json_unsigned(writer, membership.incarnation)
        && tool_json_raw(writer, ",\"record\":") && tool_json_unsigned(writer, membership.record)
        && tool_json_raw(writer, ",\"generation\":") && tool_json_unsigned(writer, membership.generation)
        && tool_json_raw(writer, "}");
}
/** Null means no mapping; no nearest source row or historical frame is fabricated. */
bool ama_debug_client_location(ToolJsonWriter *writer, const AmaProgramLocation *location) {
    if (location == NULL) { return tool_json_raw(writer, "null"); }
    return tool_json_raw(writer, "{\"operation\":") && tool_json_unsigned(writer, location->operation)
        && tool_json_raw(writer, ",\"path\":")
        && (location->path == NULL ? tool_json_raw(writer, "null")
            : tool_json_bytes(writer, (const unsigned char *)location->path, strlen(location->path)))
        && tool_json_raw(writer, ",\"line\":") && tool_json_unsigned(writer, location->line)
        && tool_json_raw(writer, ",\"column\":") && tool_json_unsigned(writer, location->column)
        && tool_json_raw(writer, ",\"role\":") && tool_json_string(writer, location->role)
        && tool_json_raw(writer, ",\"type\":") && tool_json_string(writer, location->type)
        && tool_json_raw(writer, "}");
}
/** Nested arrays use independent callback state rather than sharing mutable separator globals. */
bool ama_debug_client_next(AmaDebugClientOutput *output) {
    if (!output->first && !tool_json_raw(output->writer, ",")) { return false; }
    output->first = false;
    return true;
}
