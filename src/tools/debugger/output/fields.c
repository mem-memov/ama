/** @file
 * @brief Stream declared fields and current list occurrences without acquiring or executing targets.
 * [LIST-13](../../../../spec/language/lists.md#list-13) distinguishes stable occurrences
 * from positions; [DEBUG-03](../../../../spec/implementation/diagnostics.md#debug-03) exposes current state only.
 */
#include "../internal.h"
/** Preserve every occurrence, including empty/stale/repeated targets; inspection never dereferences them. */
static bool element(void *context, const AmaDebugElement *value) {
    AmaDebugClientOutput *output = context;
    ToolJsonWriter *writer = output->writer;
    return ama_debug_client_next(output) && tool_json_raw(writer, "{\"position\":")
        && tool_json_unsigned(writer, value->position) && tool_json_raw(writer, ",\"membership\":")
        && ama_debug_client_membership(writer, output->instance.context, value->membership)
        && tool_json_raw(writer, ",\"target\":") && ama_debug_client_reference(writer, value->target)
        && tool_json_raw(writer, "}");
}
/** The field descriptor determines list form; scalar fields are not probed as lists. */
bool ama_debug_client_field(void *context, const AmaDebugFieldValue *field) {
    AmaDebugClientOutput *output = context;
    ToolJsonWriter *writer = output->writer;
    bool list = field->form == AMA_FIELD_OWNING_LIST || field->form == AMA_FIELD_REFERENCE_LIST;
    if (!ama_debug_client_next(output) || !tool_json_raw(writer, "{\"name\":")
        || !tool_json_string(writer, field->name) || !tool_json_raw(writer, ",\"field\":")
        || !tool_json_unsigned(writer, field->field) || !tool_json_raw(writer, ",\"role\":")
        || !tool_json_unsigned(writer, field->role) || !tool_json_raw(writer, ",\"position\":")
        || !tool_json_unsigned(writer, field->position) || !tool_json_raw(writer, ",\"type\":")
        || !tool_json_unsigned(writer, field->type_id) || !tool_json_raw(writer, ",\"form\":")
        || !tool_json_unsigned(writer, field->form) || !tool_json_raw(writer, ",\"value\":")
        || !ama_debug_client_reference(writer, field->value) || !tool_json_raw(writer, ",\"count\":")
        || !tool_json_unsigned(writer, field->count) || !tool_json_raw(writer, ",\"incarnation\":")
        || !tool_json_unsigned(writer, field->incarnation) || !tool_json_raw(writer, ",\"acquired\":")
        || !tool_json_boolean(writer, field->acquired) || !tool_json_raw(writer, ",\"elements\":")) { return false; }
    AmaStatus status = ama_status_success();
    if (list) {
        if (!tool_json_raw(writer, "[")) { return false; }
        AmaDebugClientOutput elements = {.writer=writer, .session=output->session,
            .instance=output->instance, .first=true};
        status = ama_debug_elements(output->session, output->instance, field->field, element, &elements);
        if (!tool_json_raw(writer, "]")) { return false; }
    } else if (!tool_json_raw(writer, "null")) { return false; }
    if (output->status.code == AMA_OK) { output->status = status; }
    return tool_json_raw(writer, ",\"status\":") && ama_debug_client_status(writer, status)
        && tool_json_raw(writer, "}");
}
