/** @file
 * @brief Qualify a semantic owner through the existing exact static codec, never its explanatory display string.
 * [TPL-05](../../../../spec/language/templates.md#tpl-05) makes complete owner bindings nominal identity.
 * [NAME-02](../../../../spec/implementation/compiler.md#name-02) preserves structural framing and codec refusals.
 */
#include "key.h"
#include "semantic/static/closure/plan.h"

/** Validate pointer membership before dereferencing caller-supplied owner payloads.
 * [COMP-02](../../../../spec/implementation/compiler.md#comp-02) uses resolved original identities.
 */
static bool member(const SemanticProgram *program, const SemanticType *type) {
    for (const SemanticType *candidate = program->first; candidate; candidate = candidate->next) {
        if (candidate == type) { return true; }
    }
    return false;
}

/** Preserve the codec's failure class; no digest equality or collision fallback is involved in key preparation.
 * [NAME-02](../../../../spec/implementation/compiler.md#name-02) forbids fabricated alternative keys.
 */
static IdentityStatus key_status(StaticKeyStatus status) {
    switch (status) {
        case STATIC_KEY_OK: return IDENTITY_OK;
        case STATIC_KEY_OUT_OF_MEMORY: return IDENTITY_OUT_OF_MEMORY;
        case STATIC_KEY_LIMIT: return IDENTITY_LIMIT;
        default: return IDENTITY_INVALID;
    }
}

IdentityStatus semantic_type_key(const SemanticProgram *program, const SemanticType *type,
    SourceArena *arena, QualifiedIdentity *output) {
    if (!program || program->status != SEMANTIC_OK || !program->relations_valid || !arena || !output
        || !member(program, type)) { return IDENTITY_INVALID; }
    QualifiedIdentity base = {.category = IDENTITY_TYPE, .module_path = type->module->source->logical_path};
    if (type->specialization) {
        if (!program->type_closure || !program->type_closure->closed) { return IDENTITY_INVALID; }
        StaticRegistryPlan *registry = program->type_closure->connections->constants->publications->results->registry;
        StaticKeyEncoding key;
        IdentityStatus status = key_status(static_key_encode(registry->guards->applications->terms,
            type->specialization->context->registry_use->candidate->key, arena, &key));
        if (status != IDENTITY_OK) { return status; }
        base = key.qualified;
        base.category = IDENTITY_TYPE;
    }
    *output = base;
    return IDENTITY_OK;
}
