/** @file
 * @brief Materialize exact resolved targets/members without scanning or splitting authored source names.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * permits rooted fallback operands, preserves constant selection and forbids
 * rewriting fixed implicit dependencies. Temporary joined bytes never escape.
 */
#include <string.h>
#include "rooted.h"
#include "source/path/logical.h"
SourceOperandStatus source_operand_rooted(SourceTextEdits *edits, const SourceCloneMap *map,
    const DeliverySourceDependency *edge, SourceOperandIssue *issue) {
    if (!edits || edits->attempted) { return SOURCE_OPERAND_INVALID; }
    if (issue) { *issue = (SourceOperandIssue){.edge = edge, .edits_status = edits->status}; }
    if (edits->status != SOURCE_EDITS_OK) { return SOURCE_OPERAND_EDITS; }
    if (!map || !map->ready || !edge || !issue || !edge->target
        || !module_path_canonical((const unsigned char *)edge->target, strlen(edge->target))
        || edge->origin.source != edits->original || !source_span_valid(edge->origin)
        || (!edge->implicit && edge->origin.start == edge->origin.end) || (edge->member && !*edge->member)) {
        edits->status = SOURCE_EDITS_INVALID;
        if (issue) { issue->edits_status = edits->status; }
        return SOURCE_OPERAND_INVALID;
    }
    const SourceCloneMapping *target = map->first;
    while (target && strcmp(target->original->logical_path, edge->target)) { target = target->next; }
    const char *expected = target ? target->identity : edge->target;
    issue->expected = expected;
    if (edge->implicit) {
        if (!strcmp(expected, edge->target)) { return SOURCE_OPERAND_OK; }
        edits->failed = edge->origin; issue->edits_status = edits->status = SOURCE_EDITS_CONFLICT;
        return SOURCE_OPERAND_FIXED;
    }
    SourceArena scratch; source_arena_init(&scratch, edits->arena.allocator);
    const char *operand = edge->member ? source_arena_join(&scratch, expected, "/", edge->member) : expected;
    if (!operand) { edits->failed = edge->origin; edits->status = SOURCE_EDITS_MEMORY; }
    else {
        size_t length = strlen(operand);
        if (edge->origin.end - edge->origin.start != length
            || memcmp(source_bytes(edge->origin.source) + edge->origin.start, operand, length)) {
            edits->status = source_text_edits_add(edits, edge->origin, (const unsigned char *)operand, length);
        }
    }
    source_arena_destroy(&scratch);
    issue->edits_status = edits->status;
    return edits->status == SOURCE_EDITS_OK ? SOURCE_OPERAND_OK : SOURCE_OPERAND_EDITS;
}
