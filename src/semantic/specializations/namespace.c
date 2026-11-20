/** @file
 * @brief Select constant namespaces by original syntax and complete caller key rather than source path guesses.
 * [CONST-02](../../../spec/language/declarations.md#const-02) fixes qualified source identity.
 * [CONST-03](../../../spec/language/declarations.md#const-03) closes saved read namespaces independently.
 * [TPL-05](../../../spec/language/templates.md#tpl-05) preserves full capture identity.
 */
#include "namespace.h"
#include "semantic/internal.h"
#include "semantic/static/closure/plan.h"

/** Normalize retained evidence, then require exactly one already projected
 * nominal vertex. Multiple incoming paths must agree on that identity.
 * [TPL-05](../../../spec/language/templates.md#tpl-05) shares equal-key metadata only.
 */
static bool merge(SemanticProgram *program, const StaticTerm *term,
    SourceSpan site, SemanticType **selected) {
    StaticConstraintSolver *solver = program->type_closure->connections->constants->publications->results->registry->guards->applications->solver;
    const StaticTerm *value;
    StaticConstraintOutcome outcome;
    if (!static_constraint_value(solver, term, &value, &outcome)) {
        program->status = solver->status == STATIC_CONSTRAINT_OUT_OF_MEMORY ? SEMANTIC_OUT_OF_MEMORY : SEMANTIC_INVALID;
        program->error = solver->error;
        return false;
    }
    SemanticType *match = NULL;
    if (outcome == STATIC_CONSTRAINT_COMPLETE) {
        for (SemanticType *type = program->first; type; type = type->next) {
            if (type->specialization->context->registry_use->candidate->key->owner_type == value) {
                match = type; break;
            }
        }
    }
    if (!match || (*selected && *selected != match)) {
        return semantic_fail(program, site, "Constant namespace lacks one closed required nominal identity", NULL);
    }
    *selected = match;
    return true;
}

/** Original syntax is scoped by full caller identity; distinct actual incoming
 * paths are inspected rather than using a source-module cache hit as proof.
 * [TPL-05](../../../spec/language/templates.md#tpl-05) preserves captures.
 */
static bool caller(const SemanticType *type, const StaticResultContext *context) {
    return type->specialization->context->registry_use->candidate == context->registry_use->candidate;
}

SemanticType *semantic_read_namespace(SemanticProgram *program, const SemanticType *owner, const AstNode *syntax) {
    if (!program->type_closure) {
        (void)semantic_fail(program,syntax->span,"No closed proof exists for this saved constant namespace",NULL);
        return NULL;
    }
    SemanticType *selected = NULL;
    const StaticConstantPlan *constants = program->type_closure->connections->constants;
    for (const StaticConstantSelection *item = constants->first_selection; item; item = item->next) {
        if (item->read->syntax == syntax && caller(owner, item->read->context)
            && !merge(program, item->namespace_type, syntax->span, &selected)) { return NULL; }
    }
    if (!selected) { (void)semantic_fail(program, syntax->span, "No closed proof exists for this saved constant namespace", NULL); }
    return selected;
}

SemanticType *semantic_qualified_namespace(SemanticProgram *program, const SemanticType *owner, const AstSource *syntax) {
    if (!program->type_closure) {
        (void)semantic_fail(program,syntax->span,"No closed proof exists for this qualified constant namespace",NULL);
        return NULL;
    }
    SemanticType *selected = NULL;
    const StaticConstantPlan *constants = program->type_closure->connections->constants;
    for (const StaticConstantSource *item = constants->first_source; item; item = item->next) {
        if (item->occurrence->source == syntax && caller(owner, item->context)
            && !merge(program, item->namespace_type, syntax->span, &selected)) { return NULL; }
    }
    if (!selected) { (void)semantic_fail(program, syntax->span, "No closed proof exists for this qualified constant namespace", NULL); }
    return selected;
}
