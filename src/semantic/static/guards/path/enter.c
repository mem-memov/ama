/** @file
 * @brief Check known original template ancestry before expanding a source-backed application.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) forbids repeated creation origins.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) requires output producers.
 * [COMP-04](../../../../../spec/implementation/compiler.md#comp-04) retains the full rejected path.
 */
#include "../internal.h"

/** Registered applications must originate in this expanded definition, not
 * merely in an enclosing syntactic argument. Validate pointers before access.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) fixes lexical creation ownership.
 */
static bool application(StaticGuardPlan *plan, const StaticCreationPath *parent,
    const StaticApplicationRecord *record) {
    if (static_guard_has_path(plan, parent)) {
        for (const StaticApplicationRecord *item = plan->applications->first; item; item = item->next) {
            if (item != record) { continue; }
            if (item->context.environment == parent->owner->environment && item->context.expression != NULL) { return true; }
            break;
        }
    }
    return static_guard_fail(plan, STATIC_GUARD_INVALID, (SourceSpan){0},
        "Creation application must belong to its exact lexical owner path");
}

/** Original template identity is independent of captures, aliases and cache
 * keys. Ordinary zero-parameter modules remain graph nodes but do not trigger
 * this predicate. Preserve every existing frame plus the closing application.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) checks original sources.
 */
static bool ancestry(StaticGuardPlan *plan, const StaticCreationPath *parent,
    const StaticModule *origin, SourceSpan site) {
    if (origin->parameter_count == 0) { return true; }
    for (const StaticCreationPath *ancestor = parent; ancestor; ancestor = ancestor->parent) {
        if (ancestor->owner->environment->definition != origin) { continue; }
        plan->rejected_path = parent;
        plan->repeated_ancestor = ancestor;
        plan->rejected_origin = origin;
        plan->rejected_site = site;
        (void)static_guard_fail(plan, STATIC_GUARD_CONFLICT, site, "Repeated original template on a specialization-creation path");
        const ProjectModule *definition = origin->definition;
        plan->related[0] = (DiagnosticRelated){.location = {.span = {definition->source->text, 0, 0}},
            .role = "repeated source template", .name = definition->source->logical_path};
        plan->related[1] = (DiagnosticRelated){.location = {.span = ancestor->site},
            .role = "earlier creation on this path", .name = definition->source->logical_path};
        plan->error.related = plan->related;
        plan->error.related_count = 2;
        return false;
    }
    return true;
}

/** Classify already checked structural values. Unapplied constructors and plain
 * supplied Types carry no new edge, while every explicit output phase requires
 * its producer even if its head was a Type parameter or ordinary module.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) overrides plain reuse.
 */
static StaticCreationOutcome classify(StaticGuardPlan *plan, const StaticApplicationRecord *record,
    const StaticTerm **value) {
    if (record->pending_origin || record->value == NULL) { return STATIC_CREATION_PENDING; }
    StaticCategory category;
    const StaticSignature *signature;
    static_application_description(plan->applications, record->value, &category, &signature);
    (void)signature;
    if (category == STATIC_CATEGORY_TEMPLATE) { return STATIC_CREATION_METADATA; }
    if (category == STATIC_CATEGORY_DEFERRED) { return STATIC_CREATION_PENDING; }
    bool producer = record->context.policy == STATIC_OMIT_PRODUCER;
    if (!producer && !record->applied && record->context.expression->kind == AST_TYPE_PLACEHOLDER) {
        return STATIC_CREATION_REUSE;
    }
    *value = static_constraint_root(plan->applications->solver, record->value);
    return (*value)->kind == STATIC_TERM_NOMINAL ? STATIC_CREATION_ENTERED : STATIC_CREATION_PENDING;
}

/** The source guard precedes target environment preparation. Symbolic captures
 * remain intact, so checking cannot be delayed until a growing canonical key
 * exists. Non-entered successful outcomes never publish a path handle.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) guards before expansion.
 */
bool static_creation_enter(StaticGuardPlan *plan, const StaticCreationPath *parent,
    const StaticApplicationRecord *record, const StaticCreationPath **output, StaticCreationOutcome *outcome) {
    if (!static_guard_dependencies(plan)) { return false; }
    if (output == NULL || outcome == NULL || !application(plan, parent, record)) {
        return static_guard_fail(plan, STATIC_GUARD_INVALID, (SourceSpan){0}, "Missing creation path or outcome receiver");
    }
    StaticApplicationOutcome checked;
    if (!static_application_check(plan->applications, record, &checked)) {
        (void)static_guard_dependencies(plan); return false;
    }
    const StaticTerm *value = NULL;
    StaticCreationOutcome result = classify(plan, record, &value);
    if (result != STATIC_CREATION_ENTERED) { *outcome = result; return true; }
    SourceSpan site = record->context.expression->span;
    if (!ancestry(plan, parent, value->as.nominal.origin, site)) { return false; }
    const StaticTermEnvironment *environment;
    if (!static_term_environment(plan->applications->terms, value->as.nominal.origin, value, &environment)) {
        (void)static_guard_dependencies(plan); return false;
    }
    const StaticGuardOwner *owner;
    const StaticCreationPath *path;
    if (!static_guard_owner(plan, environment, &owner) ||
        !static_guard_path(plan, parent, owner, record, site, &path)) { return false; }
    StaticGuardClosure closure;
    if (!static_guard_refresh(plan, &closure)) { return false; }
    *output = path;
    *outcome = result;
    return true;
}
