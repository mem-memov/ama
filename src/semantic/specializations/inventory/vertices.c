/** @file
 * @brief Register concrete nominal vertices before linking their potentially cyclic fields.
 * [TPL-05](../../../../spec/language/templates.md#tpl-05) shares equal full-key metadata.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) separates identity and admission.
 */
#include "internal.h"

SemanticType *semantic_specialization_candidate(const SemanticProgram *program,
    const StaticRegistryCandidate *candidate) {
    for (SemanticType *type = program->first; type; type = type->next) {
        if (type->specialization->context->registry_use->candidate == candidate) { return type; }
    }
    return NULL;
}

bool semantic_specialization_vertex(SemanticProgram *program, const StaticClosureEntry *entry) {
    const StaticRegistryCandidate *candidate = entry->context->registry_use->candidate;
    if (!candidate || !candidate->key->owner_type || !candidate->key->owner_type->ground) {
        return semantic_fail(program, entry->context->creation->site,
            "Ordinary semantic inventory requires a closed nominal specialization", NULL);
    }
    if (semantic_specialization_candidate(program, candidate)) { return true; }
    SemanticType *type = semantic_allocate(program, sizeof(*type));
    if (!type) { return false; }
    const ProjectModule *module = candidate->key->definition->definition;
    *type = (SemanticType){.module = module, .specialization = entry,
        .executable = module->syntax.sections.kind == MODULE_AMA || module->syntax.sections.kind == MODULE_NATIVE};
    abi_table_init(&type->abi, program->arena.allocator);
    if (program->last) { program->last->next = type; }
    else { program->first = type; }
    program->last = type;
    return true;
}
