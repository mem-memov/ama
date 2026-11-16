/** @file
 * @brief Stream actual live instance controls and nested declared state under one inspection lease.
 * [DEBUG-03](../../../../spec/implementation/diagnostics.md#debug-03) separates saved
 * positions from transient caller links and rejects fabricated history.
 */
#include "../internal.h"
/** The caller link is present metadata, never a reconstructed historical call frame. Component contract: [DEBUG-03](../../../../spec/implementation/diagnostics.md#debug-03). */
static bool caller(ToolJsonWriter *writer, uint64_t context, AmaReturnLink link) {
    return tool_json_raw(writer, "{\"attached\":") && tool_json_boolean(writer, link.attached)
        && tool_json_raw(writer, ",\"host\":") && tool_json_boolean(writer, link.host)
        && tool_json_raw(writer, ",\"caller\":")
        && ama_debug_client_reference(writer, (AmaContextReference){context, link.caller})
        && tool_json_raw(writer, ",\"continuation\":") && tool_json_unsigned(writer, link.continuation)
        && tool_json_raw(writer, ",\"call_site\":") && tool_json_unsigned(writer, link.call_site)
        && tool_json_raw(writer, "}");
}
/** Each nested array keeps its own separator/error state while borrowing the same exclusive session. Component contract: [DEBUG-03](../../../../spec/implementation/diagnostics.md#debug-03). */
static bool declarations(AmaDebugClientOutput *output, const AmaDebugInstance *instance) {
    ToolJsonWriter *writer = output->writer;
    AmaDebugClientOutput fields = {.writer=writer, .session=output->session,
        .instance=instance->reference, .first=true};
    if (!tool_json_raw(writer, ",\"fields\":[")) { return false; }
    AmaStatus status = ama_debug_fields(output->session, instance->reference, ama_debug_client_field, &fields);
    if (status.code == AMA_OK) { status = fields.status; }
    if (output->status.code == AMA_OK) { output->status = status; }
    if (!tool_json_raw(writer, "],\"fields_status\":") || !ama_debug_client_status(writer, status)
        || !tool_json_raw(writer, ",\"cursors\":[")) { return false; }
    AmaDebugClientOutput cursors = {.writer=writer, .session=output->session,
        .instance=instance->reference, .first=true};
    status = ama_debug_cursors(output->session, instance->reference, ama_debug_client_cursor, &cursors);
    if (output->status.code == AMA_OK) { output->status = status; }
    return tool_json_raw(writer, "],\"cursors_status\":") && ama_debug_client_status(writer, status)
        && tool_json_raw(writer, ",\"native\":") && ama_debug_client_native(output, instance);
}
/** Stream exactly one currently published instance; never recurse through ownership or target references. Component contract: [DEBUG-03](../../../../spec/implementation/diagnostics.md#debug-03). */
bool ama_debug_client_instance(void *context, const AmaDebugInstance *instance) {
    AmaDebugClientOutput *output = context;
    ToolJsonWriter *writer = output->writer;
    return ama_debug_client_next(output) && tool_json_raw(writer, "{\"reference\":")
        && ama_debug_client_reference(writer, instance->reference) && tool_json_raw(writer, ",\"owner\":")
        && ama_debug_client_reference(writer, instance->owner) && tool_json_raw(writer, ",\"type_name\":")
        && tool_json_string(writer, instance->type_name) && tool_json_raw(writer, ",\"activity\":")
        && tool_json_unsigned(writer, instance->activity) && tool_json_raw(writer, ",\"lifetime\":")
        && tool_json_unsigned(writer, instance->lifetime) && tool_json_raw(writer, ",\"resume_valid\":")
        && tool_json_boolean(writer, instance->resume.valid) && tool_json_raw(writer, ",\"resume_selector\":")
        && tool_json_unsigned(writer, instance->resume.selector) && tool_json_raw(writer, ",\"resume_location\":")
        && ama_debug_client_location(writer, instance->resume_location) && tool_json_raw(writer, ",\"caller\":")
        && caller(writer, instance->reference.context, instance->caller) && tool_json_raw(writer, ",\"outgoing\":")
        && ama_debug_client_reference(writer, instance->outgoing) && tool_json_raw(writer, ",\"recorded_operation\":")
        && tool_json_unsigned(writer, instance->recorded_operation) && tool_json_raw(writer, ",\"recorded_location\":")
        && ama_debug_client_location(writer, instance->recorded_location) && tool_json_raw(writer, ",\"last_refusal\":")
        && ama_debug_client_status(writer, instance->last_refusal) && tool_json_raw(writer, ",\"native_opaque\":")
        && tool_json_boolean(writer, instance->native_opaque) && declarations(output, instance)
        && tool_json_raw(writer, "}");
}
