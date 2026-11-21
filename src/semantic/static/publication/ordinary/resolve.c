/** @file
 * @brief Require complete ordinary Type metadata while retaining its original publication proof.
 * [TYPE-01](../../../../../spec/language/declarations.md#type-01) excludes residual constructor storage.
 * [CONST-03](../../../../../spec/language/declarations.md#const-03) fixes namespaces before member selection.
 * [TPL-14](../../../../../spec/language/static-results.md#tpl-14) excludes marker
 * origins even after placeholder substitution or export selection.
 */
#include "internal.h"
#include "projections/internal.h"

/** Ordinary namespace resolution preserves actual source provenance. Source-only
 * selections close their attributed supplier without inventing an ordinary root.
 * Neither path selects a Type from member names.
 * [CONST-03](../../../../../spec/language/declarations.md#const-03) forbids namespace guessing.
 */
bool static_ordinary_resolve(StaticPublicationPlan *plan, StaticOrdinaryType *type,
    const StaticTerm **output) {
    if (!static_publication_dependencies(plan)) { return false; }
    if (!output || !static_ordinary_has(plan, type) || (!type->context && !type->producer)) {
        return static_publication_fail(plan, STATIC_PUBLICATION_INVALID, (SourceSpan){0}, "Invalid ordinary Type resolution handle or receiver");
    }
    bool changed;
    if (!static_ordinary_equations(plan, type, &changed)) { return false; }
    if (type->context) {
        const StaticPublication *publication;
        if (!static_publication_resolve(plan, type->context, type->expression, &publication)) { return false; }
        type->publication = publication;
    } else {
        if (!type->projection || !type->projection->context) {
            return static_publication_fail(plan, STATIC_PUBLICATION_UNRESOLVED, type->expression->span,
                "The selected supplier remains unresolved under its lexical creation ancestry");
        }
        const StaticPublication *publication;
        if (!static_publication_scope(plan, type->projection->context, &publication) ||
            !static_ordinary_equations(plan, type, &changed)) { return false; }
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
    if (value->as.nominal.origin->definition->syntax.sections.kind == MODULE_MARKER) {
        return static_publication_fail(plan, STATIC_PUBLICATION_CONFLICT, type->expression->span,
            "A static marker Type has no runtime storage or constant namespace");
    }
    *output = value;
    return true;
}
