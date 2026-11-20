/** @file
 * @brief Describe original ordinary or static declarations without manufacturing a concrete specialization.
 * [NAME-06](../../../../spec/language/declarations.md#name-06) preserves lexical source identity.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) gives static interfaces metadata rather than runtime slots.
 */
#include "internal.h"

/** Role order counts only actual persistent source fields. Locals have no
 * external interface position and are represented explicitly without one.
 * [TYPE-06](../../../../spec/language/declarations.md#type-06) supplies the original field inventory before Type substitution.
 */
static bool position(ToolJsonWriter *writer, const EditorDefinition *value) {
    if (value->original->scope) { return tool_json_raw(writer,"null"); }
    size_t ordinal = 0;
    if (value->runtime) {
        for (const StaticRuntimeField *field = value->runtime->first_field; field; field = field->next) {
            if (field == value->original) { return tool_json_unsigned(writer,ordinal); }
            if (!field->scope && field->role == value->original->role) { ordinal++; }
        }
    }
    writer->status = TOOL_JSON_INVALID_VALUE;
    return false;
}

bool driver_editor_declaration(ToolJsonWriter *writer, const EditorDefinition *value) {
    if (value->symbol) { return tool_source_symbol(writer,value->symbol); }
    if (value->static_symbol) {
        const StaticSymbol *symbol = value->static_symbol;
        const char *roles[] = {"type-parameter","type-result","type-alias"};
        const char *categories[] = {"deferred","type","template"};
        if ((unsigned)symbol->role >= sizeof(roles)/sizeof(*roles) ||
            (unsigned)symbol->category >= sizeof(categories)/sizeof(*categories)) {
            writer->status = TOOL_JSON_INVALID_VALUE;
            return false;
        }
        return tool_json_raw(writer,"{\"name\":") && tool_source_spelling(writer,symbol->name)
            && tool_json_raw(writer,",\"owner\":") && tool_json_string(writer,symbol->owner->definition->source->logical_path)
            && tool_json_raw(writer,",\"role\":") && tool_json_string(writer,roles[symbol->role])
            && tool_json_raw(writer,",\"category\":") && tool_json_string(writer,categories[symbol->category])
            && tool_json_raw(writer,",\"declaration\":") && tool_source_span(writer,symbol->site) && tool_json_raw(writer,"}");
    }
    if (!value->original) { return tool_json_raw(writer,"null"); }
    const StaticRuntimeField *field = value->original;
    const char *roles[] = {"constant","parameter","result","state"};
    if (!value->module || (unsigned)field->role >= sizeof(roles)/sizeof(*roles)) {
        writer->status = TOOL_JSON_INVALID_VALUE;
        return false;
    }
    return tool_json_raw(writer,"{\"name\":") && tool_source_spelling(writer,field->name)
        && tool_json_raw(writer,",\"owner\":") && tool_json_string(writer,value->module->source->logical_path)
        && tool_json_raw(writer,",\"type\":") && tool_source_type(writer,value->type)
        && tool_json_raw(writer,",\"role\":") && tool_json_string(writer,roles[field->role])
        && tool_json_raw(writer,",\"position\":") && position(writer,value)
        && tool_json_raw(writer,",\"reference\":") && tool_json_boolean(writer,field->reference)
        && tool_json_raw(writer,",\"list\":") && tool_json_boolean(writer,field->list)
        && tool_json_raw(writer,",\"local\":") && tool_json_boolean(writer,field->scope != NULL)
        && tool_json_raw(writer,",\"inferred\":") && tool_json_boolean(writer,field->inferred)
        && tool_json_raw(writer,",\"declaration\":") && tool_source_span(writer,field->name) && tool_json_raw(writer,"}");
}
