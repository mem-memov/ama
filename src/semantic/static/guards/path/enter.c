/** @file
 * @brief Classify source-backed creation and resolve repeated keys before expanding definitions.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) permits exact-key cycles and suspends unknown repeats.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) keeps graph producers
 * separate from ordinary nominal applications.
 * [COMP-04](../../../../../spec/implementation/compiler.md#comp-04) retains the full rejected path.
 * [TPL-14](../../../../../spec/language/static-results.md#tpl-14) makes a marker
 * an already closed identity with no body or instance to expand.
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

/** Classify ordinary constructor values only. Separate static producers do not
 * enter this nominal application path. Bare supplied Types and closed ordinary
 * constant namespaces reuse their known identity without creating a new edge.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) separates creation
 * from reuse; [TPL-12](../../../../../spec/language/static-results.md#tpl-12)
 * keeps graph producers outside ordinary Type/Template values.
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
    if (!record->applied && record->context.expression->kind == AST_TYPE_PLACEHOLDER) {
        return STATIC_CREATION_REUSE;
    }
    *value = static_constraint_root(plan->applications->solver, record->value);
    if ((*value)->kind == STATIC_TERM_NOMINAL
        && (*value)->as.nominal.origin->definition->syntax.sections.kind == MODULE_MARKER) {
        return STATIC_CREATION_REUSE;
    }
    if (record->context.policy == STATIC_OMIT_NAMESPACE &&
        (*value)->kind == STATIC_TERM_NOMINAL && (*value)->as.nominal.origin->parameter_count == 0) {
        return STATIC_CREATION_REUSE;
    }
    return (*value)->kind == STATIC_TERM_NOMINAL ? STATIC_CREATION_ENTERED : STATIC_CREATION_PENDING;
}

/** Repeated unresolved origins stop before target scope preparation; exact
 * closed repeats can register an edge to the existing nominal vertex.
 * Non-entered successful outcomes never publish a path handle.
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
    const StaticCreationPath *path = NULL;
    for (const StaticCreationPath *item = plan->first_path; item; item = item->next) {
        if (item->parent == parent && item->application == record) { path = item; break; }
    }
    if (path == NULL) {
        SourceSpan site = record->context.expression->span;
        bool pending;
        if (!static_guard_ancestry(plan, parent, value, site, &pending)) { return false; }
        if (pending) { *outcome = STATIC_CREATION_PENDING; return true; }
        const StaticTerm *closed;
        StaticConstraintOutcome normalization;
        if (!static_constraint_value(plan->applications->solver, value, &closed, &normalization)) {
            (void)static_guard_dependencies(plan); return false;
        }
        if (normalization == STATIC_CONSTRAINT_COMPLETE) { value = closed; }
        const StaticTermEnvironment *environment;
        if (!static_term_environment(plan->applications->terms, value->as.nominal.origin, value, &environment)) {
            (void)static_guard_dependencies(plan); return false;
        }
        const StaticGuardOwner *owner;
        if (!static_guard_owner(plan, environment, &owner) ||
            !static_guard_path(plan, parent, owner, record, site, &path)) { return false; }
    }
    StaticGuardClosure closure;
    if (!static_guard_refresh(plan, &closure)) { return false; }
    *output = path;
    *outcome = result;
    return true;
}
