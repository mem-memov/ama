/** @file
 * @brief Require complete ordinary Type metadata while retaining its original publication proof.
 * [TYPE-01](../../../../../spec/language/declarations.md#type-01) excludes residual constructor storage.
 * [CONST-03](../../../../../spec/language/declarations.md#const-03) fixes namespaces before member selection.
 */
#include "internal.h"

/** Ordinary namespace resolution preserves source provenance; exports retain
 * their independent supplier root. Neither path selects a Type from member names.
 * [CONST-03](../../../../../spec/language/declarations.md#const-03) forbids namespace guessing.
 */
bool static_ordinary_resolve(StaticPublicationPlan *plan, StaticOrdinaryType *type,
    const StaticTerm **output) {
    if (!static_publication_dependencies(plan)) { return false; }
    if (!output || !static_ordinary_has(plan, type) || (!type->context && !type->producer)) {
        return static_publication_fail(plan, STATIC_PUBLICATION_INVALID, (SourceSpan){0}, "Invalid ordinary Type resolution handle or receiver");
    }
    if (type->producer) {
        if (!static_ordinary_export(plan, type)) { return false; }
    } else {
        const StaticPublication *publication;
        if (!static_publication_resolve(plan, type->context, type->expression, &publication)) { return false; }
        type->publication = publication;
    }
    const StaticTerm *value;
    StaticConstraintOutcome outcome;
    if (!static_constraint_value(plan->results->guards->applications->solver, type->value, &value, &outcome)) {
        (void)static_publication_dependencies(plan); return false;
    }
    if (outcome != STATIC_CONSTRAINT_COMPLETE) {
        return static_publication_fail(plan, STATIC_PUBLICATION_UNRESOLVED, type->expression->span,
            "The ordinary Type remains unresolved");
    }
    if (value->kind != STATIC_TERM_NOMINAL || value->category != STATIC_CATEGORY_TYPE) {
        return static_publication_fail(plan, STATIC_PUBLICATION_CONFLICT, type->expression->span,
            "Ordinary declarations and namespaces require a complete Type");
    }
    *output = value;
    return true;
}
