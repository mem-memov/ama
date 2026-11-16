/** @file
 * @brief Describe retained traversal roles and numerical boundaries without activating absent locals.
 * [FLOW-05](../../../../spec/language/lists.md#flow-05) keeps lexical absence explicit;
 * [LIST-13](../../../../spec/language/lists.md#list-13) defines live traversal boundaries.
 */
#include "../internal.h"
/** Inactive values remain the runtime's explicit zero observations, never reconstructed bindings. Component contract: [FLOW-05](../../../../spec/language/lists.md#flow-05), [LIST-13](../../../../spec/language/lists.md#list-13). */
bool ama_debug_client_cursor(void *context, const AmaDebugCursor *cursor) {
    AmaDebugClientOutput *output = context;
    ToolJsonWriter *writer = output->writer;
    return ama_debug_client_next(output) && tool_json_raw(writer, "{\"scope\":")
        && tool_json_unsigned(writer, cursor->scope) && tool_json_raw(writer, ",\"binding_name\":")
        && tool_json_string(writer, cursor->binding_name) && tool_json_raw(writer, ",\"local\":")
        && tool_json_boolean(writer, cursor->local) && tool_json_raw(writer, ",\"active\":")
        && tool_json_boolean(writer, cursor->active) && tool_json_raw(writer, ",\"role_active\":")
        && tool_json_boolean(writer, cursor->role_active) && tool_json_raw(writer, ",\"boundary\":")
        && tool_json_unsigned(writer, cursor->boundary) && tool_json_raw(writer, ",\"collection_owner\":")
        && ama_debug_client_reference(writer, cursor->collection_owner) && tool_json_raw(writer, ",\"collection_field\":")
        && tool_json_unsigned(writer, cursor->collection_field) && tool_json_raw(writer, ",\"incarnation\":")
        && tool_json_unsigned(writer, cursor->incarnation) && tool_json_raw(writer, ",\"current\":")
        && ama_debug_client_membership(writer, output->instance.context, cursor->current)
        && tool_json_raw(writer, ",\"target\":") && ama_debug_client_reference(writer, cursor->target)
        && tool_json_raw(writer, ",\"binding_roles\":") && tool_json_unsigned(writer, cursor->binding_roles)
        && tool_json_raw(writer, ",\"associated\":") && tool_json_boolean(writer, cursor->associated)
        && tool_json_raw(writer, ",\"value\":") && ama_debug_client_reference(writer, cursor->value)
        && tool_json_raw(writer, ",\"header\":") && tool_json_unsigned(writer, cursor->header)
        && tool_json_raw(writer, ",\"repeat\":") && tool_json_unsigned(writer, cursor->repeat)
        && tool_json_raw(writer, "}");
}
