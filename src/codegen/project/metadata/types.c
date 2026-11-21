/** @file
 * @brief Serialize checked nominal Types and declaration fields as a finite graph.
 * [NAME-02](../../../../spec/implementation/compiler.md#name-02) preserves full
 * substitution/capture keys; [COMP-02](../../../../spec/implementation/compiler.md#comp-02)
 * requires consuming resolved field identities without resolving source again.
 */
#include "internal.h"
/** Emit exact source name, role, shape and target graph edge, never a C offset.
 * [COMP-02](../../../../spec/implementation/compiler.md#comp-02) retains logical fields.
 */
static bool field(ToolJsonWriter *writer, const NativeScaffold *scaffold, const StorageField *entry) {
    const SemanticSymbol *symbol = entry->symbol;
    SourceSpan name = symbol->name;
    size_t target = codegen_type_id(scaffold, native_scaffold_type(scaffold, symbol->type));
    const char *role;
    switch (symbol->role) {
    case SYMBOL_PARAMETER: role = "parameter"; break;
    case SYMBOL_RESULT: role = "result"; break;
    case SYMBOL_STATE: role = "state"; break;
    default: return false;
    }
    return target != 0 && tool_json_raw(writer, "{\"name\":")
        && tool_json_text(writer, source_bytes(name.source) + name.start, name.end - name.start)
        && tool_json_raw(writer, ",\"role\":") && tool_json_string(writer, role)
        && tool_json_raw(writer, ",\"ordinal\":") && tool_json_unsigned(writer, symbol->role_ordinal)
        && tool_json_raw(writer, ",\"type\":") && tool_json_unsigned(writer, target)
        && tool_json_raw(writer, ",\"reference\":") && tool_json_boolean(writer, symbol->reference)
        && tool_json_raw(writer, ",\"list\":") && tool_json_boolean(writer, symbol->list)
        && tool_json_raw(writer, "}");
}
bool delivery_metadata_types(ToolJsonWriter *writer, const NativeScaffold *scaffold) {
    if (!tool_json_raw(writer, "[")) { return false; }
    bool first = true;
    for (const ScaffoldType *node = scaffold->types; node; node = node->next) {
        if ((!first && !tool_json_raw(writer, ",")) || !tool_json_raw(writer, "{\"id\":")
            || !tool_json_unsigned(writer, codegen_type_id(scaffold, node))
            || !tool_json_raw(writer, ",\"module\":")
            || !tool_json_string(writer, node->plan->type->module->source->logical_path)
            || !tool_json_raw(writer, ",\"key\":")
            || !delivery_metadata_bytes(writer, node->native_prefix->encoding, node->native_prefix->encoding_length)
            || !tool_json_raw(writer, ",\"fields\":[")) { return false; }
        bool first_field = true;
        for (const StorageField *entry = node->plan->first; entry; entry = entry->next) {
            if ((!first_field && !tool_json_raw(writer, ",")) || !field(writer, scaffold, entry)) { return false; }
            first_field = false;
        }
        if (!tool_json_raw(writer, "]}")) { return false; }
        first = false;
    }
    return tool_json_raw(writer, "]");
}
