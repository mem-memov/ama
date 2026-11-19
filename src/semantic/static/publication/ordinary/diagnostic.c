/** @file
 * @brief Preserve the real ordinary selector requiring an independently checked static producer.
 * [COMP-04](../../../../../spec/implementation/compiler.md#comp-04) retains original definition/application provenance.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) keeps diagnostic attribution separate from supplier inference and creation ancestry.
 */
#include <assert.h>
#include "internal.h"

void static_ordinary_error_context(StaticPublicationPlan *plan, StaticOrdinaryType *type) {
    DiagnosticRelated related = {{.span = type->expression->selected_export},
        "ordinary declaration requiring this static export", type->producer->definition->source->logical_path};
    bool added = diagnostic_add_context(&plan->error, &type->error_context, related);
    assert(added);
    (void)added;
}

/** The first actual residual in canonical formal order proves the value remains
 * Template. Preserve the selected definition and original refusal, without
 * enumerating guessed arguments or changing any returned metadata.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) preserves actual residuals.
 * [COMP-04](../../../../../spec/implementation/compiler.md#comp-04) retains original source witnesses.
 */
bool static_ordinary_export_failure(StaticPublicationPlan *plan, StaticOrdinaryType *type,
    StaticPublicationStatus status, const char *message, const StaticTerm *value) {
    if (plan->status != STATIC_PUBLICATION_OK) { return false; }
    (void)static_publication_fail(plan, status, type->expression->selected_export, message);
    DiagnosticRelated related = {{.span = type->selected->site},
        "definition of the selected static export", type->producer->definition->source->logical_path};
    bool added = diagnostic_add_context(&plan->error, &type->error_context, related);
    assert(added);
    (void)added;
    if (value && value->kind == STATIC_TERM_NOMINAL && value->as.nominal.first_residual) {
        const StaticSymbol *formal = value->as.nominal.first_residual->formal;
        related = (DiagnosticRelated){{.span = formal->site},
            "unfilled constructor formal", formal->owner->definition->source->logical_path};
        added = diagnostic_add_context(&plan->error, &type->residual_context, related);
        assert(added);
        (void)added;
    }
    return false;
}
