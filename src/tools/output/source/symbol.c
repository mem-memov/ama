/** @file
 * @brief Describe checked source declarations without exposing generated identifiers or runtime instance values.
 * [NAME-06](../../../../spec/language/declarations.md#name-06)
 * distinguishes lexical declarations even when their names are equal.
 */
#include "source.h"
/** Intrinsic interfaces lack a nominal type; represent absence rather than creating a fake source module. Component contract: [NAME-06](../../../../spec/language/declarations.md#name-06). */
bool tool_source_type(ToolJsonWriter *writer, const SemanticType *type) {
    if (type == NULL) { return tool_json_raw(writer,"null"); }
    return tool_json_string(writer,type->module->source->logical_path);
}
/** Declaration role and reference/list shape come directly from the checked symbol inventory. Component contract: [NAME-06](../../../../spec/language/declarations.md#name-06). */
bool tool_source_symbol(ToolJsonWriter *writer, const SemanticSymbol *symbol) {
    if (symbol == NULL) { return tool_json_raw(writer,"null"); }
    const char *roles[] = {"constant","parameter","result","state"};
    if ((unsigned)symbol->role >= sizeof(roles)/sizeof(*roles)) {
        if (writer->status == TOOL_JSON_OK) { writer->status = TOOL_JSON_INVALID_VALUE; }
        return false;
    }
    return tool_json_raw(writer,"{\"name\":") && tool_source_spelling(writer,symbol->name)
        && tool_json_raw(writer,",\"owner\":") && tool_source_type(writer,symbol->owner)
        && tool_json_raw(writer,",\"type\":") && tool_source_type(writer,symbol->type)
        && tool_json_raw(writer,",\"role\":") && tool_json_string(writer,roles[symbol->role])
        && tool_json_raw(writer,",\"position\":") && tool_json_unsigned(writer,symbol->role_ordinal)
        && tool_json_raw(writer,",\"reference\":") && tool_json_boolean(writer,symbol->reference)
        && tool_json_raw(writer,",\"list\":") && tool_json_boolean(writer,symbol->list)
        && tool_json_raw(writer,",\"local\":") && tool_json_boolean(writer,symbol->local)
        && tool_json_raw(writer,",\"inferred\":") && tool_json_boolean(writer,symbol->inferred)
        && tool_json_raw(writer,",\"declaration\":") && tool_source_span(writer,symbol->name) && tool_json_raw(writer,"}");
}
