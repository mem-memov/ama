/** @file
 * @brief Retain first JSON failures and consume only standard JSON whitespace.
 * [PROJECT-01](../../../spec/implementation/project-tools.md#project-01)
 * shares immutable protocol syntax without sharing project mutation or compiler owners;
 * [COMP-04](../../../spec/implementation/compiler.md#comp-04) requires
 * exact bytes and source coordinates rather than lossy scalar conversions;
 * [ARCH-03](../../../spec/implementation/compiler.md#arch-03) retains
 * partial trees and decoded strings under one explicit operation arena.
 */
#include "internal.h"
bool source_json_fail(SourceJsonCursor *cursor, SourceJsonStatus status, const char *message) {
    if (cursor->status == SOURCE_JSON_OK) {
        size_t end = cursor->offset < cursor->length ? cursor->offset + 1 : cursor->offset;
        cursor->issue = (SourceJsonIssue){{cursor->source, cursor->offset, end}, message};
        cursor->status = status;
    }
    return false;
}
void source_json_space(SourceJsonCursor *cursor) {
    while (cursor->offset < cursor->length) {
        unsigned char byte = cursor->bytes[cursor->offset];
        if (byte != ' ' && byte != '\t' && byte != '\r' && byte != '\n') { break; }
        cursor->offset++;
    }
}
