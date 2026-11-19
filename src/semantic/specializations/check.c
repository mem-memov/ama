/** @file
 * @brief Compose concrete ordinary checks before recording specialization-local validation evidence.
 * [TPL-06](../../../spec/language/templates.md#tpl-06) checks all concrete operations and defaults.
 * [TPL-11](../../../spec/language/templates.md#tpl-11) closes finite groups only after local checks.
 */
#include "check.h"
#include "semantic/internal.h"

/** Preserve the registry's first actual refusal; incomplete dependencies cannot
 * be turned into an executable certificate by observing known Type identities.
 * [TPL-11](../../../spec/language/templates.md#tpl-11) separates these proofs.
 */
static bool certificates(SemanticProgram *program, StaticClosurePlan *closure) {
    StaticRegistryPlan *registry = closure->connections->constants->publications->results->registry;
    for (SemanticType *type = program->first; type; type = type->next) {
        if (!static_registry_complete_local(registry, type->specialization->context->registry_use)) {
            program->status = registry->status == STATIC_REGISTRY_OUT_OF_MEMORY ? SEMANTIC_OUT_OF_MEMORY : SEMANTIC_INVALID;
            program->error = registry->error;
            return false;
        }
    }
    StaticRegistryCheckState state;
    if (!static_registry_refresh_checks(registry, &state)) {
        program->status = registry->status == STATIC_REGISTRY_OUT_OF_MEMORY ? SEMANTIC_OUT_OF_MEMORY : SEMANTIC_INVALID;
        program->error = registry->error;
        return false;
    }
    if (state != STATIC_REGISTRY_CHECKS_CLOSED) {
        SourceSpan site = closure->first ? closure->first->context->creation->site : (SourceSpan){0};
        return semantic_fail(program, site, "Required specialization validation dependencies are incomplete", NULL);
    }
    return true;
}

SemanticStatus semantic_specializations_check(SemanticProgram *program, StaticClosurePlan *closure,
    const SemanticNativeAuthoring *authoring) {
    if (semantic_specialization_names(program, closure) != SEMANTIC_OK
        || semantic_native_authoring(program, authoring) != SEMANTIC_OK
        || semantic_resolve(program) != SEMANTIC_OK
        || semantic_relations(program) != SEMANTIC_OK
        || !certificates(program, closure)) { return program->status; }
    return SEMANTIC_OK;
}
