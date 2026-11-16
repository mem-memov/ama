/** @file
 * @brief Render original half-open byte ranges without display-column conversions or guessed missing locations.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04)
 * defines one-based physical lines and byte columns, including EOF.
 */
#include <string.h>
#include "source.h"
/** Nullable C byte strings use the explicit lossless physical-name representation. Component contract: [COMP-04](../../../../spec/implementation/compiler.md#comp-04). */
bool tool_source_bytes(ToolJsonWriter *writer, const char *text) {
    return text == NULL ? tool_json_raw(writer,"null") : tool_json_bytes(writer,(const unsigned char *)text,strlen(text));
}
/** Missing spans are explicit; malformed nonempty metadata cannot become a fabricated source position. Component contract: [COMP-04](../../../../spec/implementation/compiler.md#comp-04). */
bool tool_source_span(ToolJsonWriter *writer, SourceSpan span) {
    if (span.source == NULL && span.start == 0 && span.end == 0) { return tool_json_raw(writer,"null"); }
    SourcePosition point;
    if (!source_span_valid(span) || !source_position(span.source,span.start,&point)) {
        if (writer->status == TOOL_JSON_OK) { writer->status = TOOL_JSON_INVALID_VALUE; }
        return false;
    }
    return tool_json_raw(writer,"{\"path\":") && tool_source_bytes(writer,source_path(span.source))
        && tool_json_raw(writer,",\"start\":") && tool_json_unsigned(writer,span.start)
        && tool_json_raw(writer,",\"end\":") && tool_json_unsigned(writer,span.end)
        && tool_json_raw(writer,",\"line\":") && tool_json_unsigned(writer,point.line)
        && tool_json_raw(writer,",\"column\":") && tool_json_unsigned(writer,point.column) && tool_json_raw(writer,"}");
}
/** Source names, comments and literals retain their complete original byte sequences. Component contract: [COMP-04](../../../../spec/implementation/compiler.md#comp-04). */
bool tool_source_spelling(ToolJsonWriter *writer, SourceSpan span) {
    if (span.source == NULL && span.start == 0 && span.end == 0) { return tool_json_raw(writer,"null"); }
    if (!source_span_valid(span)) {
        if (writer->status == TOOL_JSON_OK) { writer->status = TOOL_JSON_INVALID_VALUE; }
        return false;
    }
    return tool_json_bytes(writer,source_bytes(span.source)+span.start,span.end-span.start);
}
