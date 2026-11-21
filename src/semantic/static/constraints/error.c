/** @file
 * @brief Retain original constraint/definition diagnostics without allocating on failure.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves source evidence.
 * [TPL-15](../../../../spec/language/templates.md#tpl-15) retains original
 * supplier provenance for unresolved declaration selection variables.
 */
#include "internal.h"

/** An internal scalar receiver denotes a particular original foreign result.
 * Its producer is already bound in the immutable occurrence inventory. Retain
 * that actual definition instead of reporting only the caller's anonymous
 * expression variable; neither names nor candidate signatures are searched.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) attributes
 * unresolved selected Types to their responsible source producer.
 */
static const ProjectModule *selected_producer(const StaticTerm *term) {
    const StaticVariableIdentity *identity = &term->as.variable;
    if (identity->role != STATIC_VARIABLE_APPLICATION || !identity->producer ||
        !identity->producer->selected_export.source) { return NULL; }
    for (const StaticOccurrence *site = identity->environment->definition->first_occurrence; site; site = site->next) {
        if (!site->receiver && site->expression == identity->producer && site->origin &&
            site->origin->syntax.sections.kind == MODULE_STATIC) { return site->origin; }
    }
    return NULL;
}

/** Variable definitions use their original scoped formal/alias site. Nominal
 * definitions use the original module file start, never a first application
 * accidentally retained by canonical interning. An internal expression result
 * or runtime Type hole has no named formal definition; its validated original
 * lexical occurrence is retained directly as the term site.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves real sources.
 */
DiagnosticRelated static_constraint_definition(const StaticTerm *term, const char *role) {
    if (term->kind == STATIC_TERM_VARIABLE) {
        const StaticSymbol *symbol = term->as.variable.symbol;
        if (symbol == NULL) {
            const ProjectModule *producer = selected_producer(term);
            if (producer) {
                return (DiagnosticRelated){.location = {.span = {producer->source->text, 0, 0}},
                    .role = role, .name = producer->source->logical_path};
            }
            const StaticTermEnvironment *environment = term->as.variable.environment;
            return (DiagnosticRelated){.location = {.span = term->site}, .role = role,
                .name = environment->definition->definition->source->logical_path};
        }
        return (DiagnosticRelated){.location = {.span = symbol->site}, .role = role,
            .name = symbol->owner->definition->source->logical_path};
    }
    const ProjectModule *module = term->as.nominal.origin->definition;
    return (DiagnosticRelated){.location = {.span = {module->source->text, 0, 0}}, .role = role,
        .name = module->source->logical_path};
}

/** Original sites remain borrowed until solver destruction; an earlier failure
 * always wins. [COMP-04](../../../../spec/implementation/compiler.md#comp-04).
 */
bool static_constraint_failure(StaticConstraintSolver *solver, StaticConstraintStatus status,
    SourceSpan site, const char *message, const StaticTerm *left, const StaticTerm *right) {
    if (solver->status != STATIC_CONSTRAINT_OK) { return false; }
    solver->status = status;
    solver->error = (Diagnostic){.code = status == STATIC_CONSTRAINT_OUT_OF_MEMORY ?
        DIAGNOSTIC_OUT_OF_MEMORY : DIAGNOSTIC_TYPE_MISMATCH, .message = message, .primary = {.span = site}};
    const StaticTerm *terms[] = {left, right};
    const char *roles[] = {"left static definition", "right static definition"};
    for (size_t index = 0; index < 2; index++) {
        if (terms[index] == NULL) { continue; }
        solver->related[solver->error.related_count++] = static_constraint_definition(terms[index], roles[index]);
    }
    if (solver->error.related_count != 0) { solver->error.related = solver->related; }
    return false;
}
