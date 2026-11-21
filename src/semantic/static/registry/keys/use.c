/** @file
 * @brief Retain actual guard paths while binding known owner keys to shared candidates.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) checks warm/cold incoming provenance.
 * [TPL-05](../../../../../spec/language/templates.md#tpl-05) keeps identity distinct from checking.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns partial use records.
 */
#include "../internal.h"

/** Pending comparison keys remain original scopes; complete keys never change
 * after successful association. Candidate creation checks no source operations.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) forbids guessed equality.
 */
bool static_registry_bind(StaticRegistryPlan *plan, StaticRegistryUse *use) {
    /* Callers have just refreshed the guard graph. A previously normalized
     * complete key is immutable; re-querying it would only duplicate scratch. */
    if (use->candidate) { return true; }
    const StaticTermEnvironment *key;
    StaticConstraintOutcome outcome;
    if (!static_guard_owner_key(plan->guards, use->path->owner, &key, &outcome)) {
        (void)static_registry_dependencies(plan); return false;
    }
    if (outcome == STATIC_CONSTRAINT_PENDING) {
        use->key_state = STATIC_REGISTRY_KEY_PENDING;
        return true;
    }
    if (!static_registry_candidate(plan, key, use->path->site, &use->candidate)) { return false; }
    use->key_state = STATIC_REGISTRY_KEY_CLOSED;
    return true;
}

/** An original published guard path is the only entry authority. Refresh actual
 * cached edges before publishing a receiver, even for an existing use handle.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) preserves every incoming check.
 */
bool static_registry_use(StaticRegistryPlan *plan, const StaticCreationPath *path,
    StaticRegistryUse **output, StaticRegistryKeyState *state) {
    if (!static_registry_dependencies(plan)) { return false; }
    if (output == NULL || state == NULL || !static_guard_has_path(plan->guards, path) || path->source_only) {
        return static_registry_fail(plan, STATIC_REGISTRY_INVALID, (SourceSpan){0}, "Invalid specialization use path or receiver");
    }
    StaticGuardClosure closure;
    if (!static_guard_refresh(plan->guards, &closure)) { (void)static_registry_dependencies(plan); return false; }
    StaticRegistryUse *use;
    if (!static_registry_retain(plan, path, &use) || !static_registry_bind(plan, use)) { return false; }
    /* Published ancestry is finite and immutable. Iteration retains required
     * parent validation without growing the C stack or fabricating an edge.
     * Supplied-Type requirements retain validation attribution only; their
     * required_from link must not become a constructor-creation edge. */
    StaticRegistryUse *child = use;
    for (const StaticCreationPath *parent = path->parent ? path->parent : path->required_from;
        parent; parent = parent->parent ? parent->parent : parent->required_from) {
        if (parent->source_only) { break; }
        StaticRegistryUse *owner;
        if (!static_registry_retain(plan, parent, &owner) || !static_registry_bind(plan, owner) ||
            !static_registry_depend(plan, owner, child, child->path->site)) { return false; }
        child = owner;
    }
    *output = use;
    *state = use->key_state;
    return true;
}
