/** @file
 * @brief Propagate exact origins through scalar reference stores without inspecting native values, constructors or runtime handle bits.
 * [REF-07](../../../../spec/language/values-and-references.md#ref-07) supplies
 * effect-free handle operations; [TYPE-06](../../../../spec/language/declarations.md#type-06)
 * ensures inferred receiver storage already exists before body execution.
 */
#include "internal.h"
ReferenceOrigin *semantic_reference_origin(ReferenceOrigin *first, const SemanticSymbol *reference) {
    for (; first != NULL; first = first->next) {
        if (first->reference == reference) { return first; }
    }
    return NULL;
}
bool semantic_record_origin(SemanticProgram *program, ReferenceOrigin **first, const SemanticStatement *statement) {
    const SemanticSource *source = statement->source;
    ReferenceOrigin value = {.reference = statement->access->symbol};
    if (source->kind == AST_SELF_REFERENCE_SOURCE) {
        value.current_instance = true;
        value.site = source->site;
    } else if (source->kind == AST_ACQUIRE_SOURCE) {
        value.owned = source->access->symbol;
        value.site = source->site;
    } else {
        const ReferenceOrigin *origin = semantic_reference_origin(*first, source->access->symbol);
        if (origin != NULL) {
            value.owned = origin->owned;
            value.current_instance = origin->current_instance;
            value.site = origin->site;
        }
    }
    ReferenceOrigin *existing = semantic_reference_origin(*first, value.reference);
    if (existing == NULL) {
        if (value.owned == NULL && !value.current_instance) { return true; }
        existing = semantic_allocate(program, sizeof(*existing));
        if (existing == NULL) { return false; }
        existing->next = *first;
        *first = existing;
    }
    value.next = existing->next;
    *existing = value;
    return true;
}
