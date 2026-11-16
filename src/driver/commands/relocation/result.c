/** @file
 * @brief Keep durable relocation outcome independent of response-stream acknowledgement and report exact original source edits.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * never infers rollback from a failed output write after commit.
 */
#include "internal.h"
/** Recovery uses persisted file roles rather than applying or inventing a second set of source edits. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool driver_relocation_edits(ToolJsonWriter *writer, const RelocationReferences *references) {
    if (references == NULL || !references->ready) { return tool_json_raw(writer,"null"); }
    if (!tool_json_raw(writer,"[")) { return false; }
    bool first = true;
    for (const RelocationPathEdit *edit = references->edits; edit != NULL; edit = edit->next) {
        if ((!first && !tool_json_raw(writer,",")) || !tool_json_raw(writer,"{\"original\":")
            || !tool_source_span(writer,edit->original) || !tool_json_raw(writer,",\"replacement\":")
            || !tool_json_string(writer,edit->replacement) || !tool_json_raw(writer,"}")) { return false; }
        first = false;
    }
    return tool_json_raw(writer,"]");
}
/** Failure records remain available through the operation owner's lifetime; absent causes are explicitly null. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
static bool error(ToolJsonWriter *writer, const Diagnostic *diagnostic) {
    return diagnostic == NULL || diagnostic->message == NULL ? tool_json_raw(writer,"null")
        : tool_source_diagnostic(writer,diagnostic,false);
}
/** A response can describe an incomplete or committed operation even when process success cannot be acknowledged. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool driver_relocation_result(ToolJsonWriter *writer, const char *command, bool success, RelocationStatus status, const char *state, bool known, bool committed,
    bool finished, const RelocationReport *report, const RelocationReferences *references, const char *journal,
    const Diagnostic *primary, const Diagnostic *cleanup) {
    return tool_json_raw(writer,"{\"schema\":\"ama.relocation.v1\",\"command\":") && tool_json_string(writer,command)
        && tool_json_raw(writer,",\"success\":") && tool_json_boolean(writer,success)
        && tool_json_raw(writer,",\"status\":") && tool_json_string(writer,driver_relocation_status(status))
        && tool_json_raw(writer,",\"state\":") && tool_json_string(writer,state)
        && tool_json_raw(writer,",\"outcome_known\":") && tool_json_boolean(writer,known)
        && tool_json_raw(writer,",\"committed\":") && (known ? tool_json_boolean(writer,committed) : tool_json_raw(writer,"null"))
        && tool_json_raw(writer,",\"finished\":") && tool_json_boolean(writer,finished)
        && tool_json_raw(writer,",\"journal\":") && tool_source_bytes(writer,journal)
        && tool_json_raw(writer,",\"report\":") && driver_relocation_report(writer,report)
        && tool_json_raw(writer,",\"edits\":") && driver_relocation_edits(writer,references)
        && tool_json_raw(writer,",\"error\":") && error(writer,primary)
        && tool_json_raw(writer,",\"cleanup_error\":") && error(writer,cleanup) && tool_json_raw(writer,"}\n");
}
