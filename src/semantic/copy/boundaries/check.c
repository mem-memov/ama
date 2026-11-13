/** @file
 * @brief Reject positive same-instance and current-root/owned-descendant continuation overlap proofs before executable planning.
 * [COPY-06](../../../../spec/language/values-and-references.md#copy-06) forbids
 * overlap; [PROTO-04](../../../../spec/implementation/native-interface.md#proto-04)
 * keeps a proved handle origin unchanged during read-only index conversion.
 */
#include "internal.h"
bool semantic_origin_boundary(SemanticProgram *program, SemanticType *type,
    ReferenceOrigin *first, const SemanticStatement *statement) {
    if (statement->transfer != TRANSFER_CONTINUATION || statement->source->access == NULL
        || !statement->source->reference || statement->source->access->index != NULL) { return true; }
    const ReferenceOrigin *origin = semantic_reference_origin(first, statement->source->access->symbol);
    if (origin == NULL) { return true; }
    const SemanticAccess *receiver = statement->access;
    bool identical = origin->owned != NULL && receiver->index == NULL && receiver->symbol == origin->owned;
    bool descendant = origin->current_instance && receiver->symbol->owner == type;
    if (!identical && !descendant) { return true; }
    semantic_fail(program, statement->source->site, identical
        ? "Continuation self-copy through a known reference is forbidden"
        : "Continuation copying overlaps the current instance's owned storage", receiver->symbol);
    program->related[1] = (DiagnosticRelated){{.span = origin->site}, "reference formed here", type->module->source->logical_path};
    program->error.related_count = 2;
    return false;
}
