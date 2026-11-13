/** @file
 * @brief Check positive reference-origin proofs on contiguous lexical segments of effect-free handle stores.
 * [COPY-06](../../../../spec/language/values-and-references.md#copy-06) requires
 * proven violations to fail statically. Unknown origins, indexed identities and
 * native/call/control effects retain mandatory runtime checks, never guessed aliasing.
 * [TYPE-04](../../../../spec/language/declarations.md#type-04) initializes
 * declarations before body execution, so their written positions are not effect barriers.
 */
#include "internal.h"
/** Scalar nonindexed reference stores form/copy handles only; list/indexed transfers may convert, allocate or mutate membership metadata. */
static bool handle_store(const SemanticStatement *statement) {
    if (statement->transfer != TRANSFER_HANDLE || !statement->access->reference
        || statement->access->list || statement->access->index != NULL || statement->source->list) { return false; }
    return statement->source->kind == AST_SELF_REFERENCE_SOURCE
        || (statement->source->access != NULL && statement->source->access->index == NULL);
}
bool semantic_copy_boundaries(SemanticProgram *program, SemanticType *type) {
    ReferenceOrigin *origins = NULL;
    const AstNode *previous = NULL;
    for (const SemanticStatement *statement = type->statements; statement != NULL; statement = statement->next) {
        const AstNode *syntax = statement->syntax;
        if (previous == NULL || previous->next != syntax) { origins = NULL; }
        previous = syntax;
        if (syntax->kind == AST_DECLARATION) { continue; }
        if (syntax->kind != AST_ASSIGNMENT) { origins = NULL; continue; }
        if (!semantic_origin_boundary(program, type, origins, statement)) { return false; }
        if (handle_store(statement)) {
            if (!semantic_record_origin(program, &origins, statement)) { return false; }
        } else { origins = NULL; }
    }
    return true;
}
