/** @file
 * @brief Preserve complete original diagnostics and related locations in machine-readable source-tool responses.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04)
 * retains requesting/declared source locations instead of flattening them into guessed text coordinates.
 */
#include "source.h"
/** A filesystem-only location remains distinct from a loaded source range. Component contract: [COMP-04](../../../../spec/implementation/compiler.md#comp-04). */
static bool location(ToolJsonWriter *writer, const DiagnosticLocation *value) {
    return tool_json_raw(writer,"{\"path\":") && tool_source_bytes(writer,value->path)
        && tool_json_raw(writer,",\"span\":") && tool_source_span(writer,value->span) && tool_json_raw(writer,"}");
}
/** Stream one explicit note through the same location codec as the primary.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves flat and context-linked attribution uniformly.
 */
static bool related(ToolJsonWriter *writer, const DiagnosticRelated *value, bool *first) {
    if (!*first && !tool_json_raw(writer,",")) { return false; }
    *first = false;
    return tool_json_raw(writer,"{\"role\":") && tool_json_string(writer,value->role)
        && tool_json_raw(writer,",\"name\":") && tool_source_bytes(writer,value->name)
        && tool_json_raw(writer,",\"location\":") && location(writer,&value->location)
        && tool_json_raw(writer,"}");
}

/** Prose stays nullable; every related record retains its explicit role/name without inferring a correction. Component contract: [COMP-04](../../../../spec/implementation/compiler.md#comp-04). */
bool tool_source_diagnostic(ToolJsonWriter *writer, const Diagnostic *value, bool warning) {
    if (!diagnostic_valid(value)) {
        if (writer->status == TOOL_JSON_OK) { writer->status = TOOL_JSON_INVALID_VALUE; }
        return false;
    }
    if (!tool_json_raw(writer,"{\"code\":") || !tool_json_string(writer,diagnostic_code_name(value->code))
        || !tool_json_raw(writer,",\"warning\":") || !tool_json_boolean(writer,warning)
        || !tool_json_raw(writer,",\"message\":") || !tool_json_string(writer,value->message)
        || !tool_json_raw(writer,",\"subject\":") || !tool_source_bytes(writer,value->subject)
        || !tool_json_raw(writer,",\"location\":") || !location(writer,&value->primary)
        || !tool_json_raw(writer,",\"cause\":") || !tool_json_string(writer,value->cause)
        || !tool_json_raw(writer,",\"expected\":") || !tool_json_string(writer,value->expected)
        || !tool_json_raw(writer,",\"help\":") || !tool_json_string(writer,value->help)
        || !tool_json_raw(writer,",\"related\":[")) { return false; }
    bool first = true;
    for (size_t index = 0; index < value->related_count; index++) {
        if (!related(writer,&value->related[index],&first)) { return false; }
    }
    for (const DiagnosticContext *context = value->contexts; context; context = context->next) {
        if (!related(writer,&context->related,&first)) { return false; }
    }
    return tool_json_raw(writer,"]}");
}
