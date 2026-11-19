/** @file
 * @brief Orchestrate closed vertex registration and independent lexical-field projection.
 * [TPL-06](../../../../spec/language/templates.md#tpl-06) checks concrete substitutions ordinarily.
 * [COMP-02](../../../../spec/implementation/compiler.md#comp-02) gates complete phase outputs.
 */
#include "internal.h"

SemanticStatus semantic_specialization_names(SemanticProgram *program, StaticClosurePlan *closure) {
    if (program->status != SEMANTIC_OK) { return program->status; }
    if (program->names_attempted || program->first || program->last) {
        program->names_valid = false;
        program->type_closure = NULL;
        (void)semantic_fail(program, (SourceSpan){0}, "Specialized inventory requires a fresh semantic program", NULL);
        return program->status;
    }
    program->names_attempted = true;
    if (!closure || !closure->closed || closure->status != STATIC_CLOSURE_OK) {
        (void)semantic_fail(program, (SourceSpan){0}, "Required Type closure is not complete", NULL);
        return program->status;
    }
    StaticRegistryPlan *registry = closure->connections->constants->publications->results->registry;
    if (registry->guards->applications->terms->inventory->project != program->project) {
        (void)semantic_fail(program, (SourceSpan){0}, "Type closure belongs to another original project", NULL);
        return program->status;
    }
    for (const StaticClosureEntry *entry = closure->first; entry; entry = entry->next) {
        if (!semantic_specialization_vertex(program, entry)) { return program->status; }
    }
    semantic_specialization_order(program);
    for (SemanticType *type = program->first; type; type = type->next) {
        if (!semantic_specialization_fields(program, registry->guards->applications->solver, type)) {
            return program->status;
        }
    }
    program->names_valid = true;
    program->type_closure = closure;
    return SEMANTIC_OK;
}
