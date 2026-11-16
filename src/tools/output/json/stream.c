/** @file
 * @brief Retain checked borrowed-stream output state without hidden heap ownership.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04)
 * treats a short write or failed flush as failure, even when earlier bytes were accepted.
 */
#include <string.h>
#include "internal.h"
/** Initializing a writer never modifies or clears the caller's stream. Component contract: [COMP-04](../../../../spec/implementation/compiler.md#comp-04). */
void tool_json_init(ToolJsonWriter *writer, FILE *output) { *writer = (ToolJsonWriter){output,TOOL_JSON_OK}; }
/** The finite output prefix either advances completely or retains a permanent I/O error. Component contract: [COMP-04](../../../../spec/implementation/compiler.md#comp-04). */
bool tool_json_write(ToolJsonWriter *writer, const void *bytes, size_t length) {
    if (writer->status != TOOL_JSON_OK) { return false; }
    if (length != 0 && fwrite(bytes,1,length,writer->output) != length) {
        writer->status = TOOL_JSON_IO_ERROR;
        return false;
    }
    return true;
}
/** Only compiler-authored punctuation/keys enter this path; source data never becomes JSON grammar. Component contract: [COMP-04](../../../../spec/implementation/compiler.md#comp-04). */
bool tool_json_raw(ToolJsonWriter *writer, const char *text) { return tool_json_write(writer,text,strlen(text)); }
/** Success is acknowledged only after the borrowed stream flushes. Component contract: [COMP-04](../../../../spec/implementation/compiler.md#comp-04). */
bool tool_json_finish(ToolJsonWriter *writer) {
    if (writer->status != TOOL_JSON_OK) { return false; }
    if (fflush(writer->output) != 0) { writer->status = TOOL_JSON_IO_ERROR; return false; }
    return true;
}
