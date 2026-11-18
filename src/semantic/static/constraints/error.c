/** @file
 * @brief Retain original constraint/definition diagnostics without allocating on failure.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves source evidence.
 */
#include "internal.h"

/** Variable definitions use their original scoped formal/alias site. Nominal
 * definitions use the original module file start, never a first application
 * accidentally retained by canonical interning. An internal expression result
 * has no named formal definition; its original lexical expression is its site.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves real sources.
 */
static DiagnosticRelated definition(const StaticTerm *term, const char *role) {
    if (term->kind == STATIC_TERM_VARIABLE) {
        const StaticSymbol *symbol = term->as.variable.symbol;
        if (symbol == NULL) {
            const StaticTermEnvironment *environment = term->as.variable.environment;
            return (DiagnosticRelated){.location = {.span = term->as.variable.producer->span}, .role = role,
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
        solver->related[solver->error.related_count++] = definition(terms[index], roles[index]);
    }
    if (solver->error.related_count != 0) { solver->error.related = solver->related; }
    return false;
}
