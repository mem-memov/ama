/** @file
 * @brief Attribute required known-Type validation without inventing constructor edges or selected roots.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) separates Type reuse from creation and retains its outgoing checks.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns partial preparation.
 */
#include <stdint.h>
#include "../internal.h"

/** Intern the exact consumer/site/normalized-owner requirement. Creation ancestry
 * starts at the supplied Type, while registry validation keeps the consumer link.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) forbids extra edges from argument metadata.
 */
static bool requirement(StaticGuardPlan *plan, const StaticCreationPath *consumer,
    const StaticGuardOwner *owner, SourceSpan site, const StaticCreationPath **output) {
    for (const StaticCreationPath *item = plan->first_path; item; item = item->next) {
        if (item->required_from == consumer && item->owner == owner &&
            item->site.source == site.source && item->site.start == site.start && item->site.end == site.end) {
            *output = item; return true;
        }
    }
    if (plan->path_count == SIZE_MAX) {
        (void)static_guard_fail(plan, STATIC_GUARD_OUT_OF_MEMORY, site, "Required Type path count exceeds representable storage");
        return false;
    }
    StaticCreationPath *path = static_guard_allocate(plan, sizeof(*path));
    if (!path) { return false; }
    *path = (StaticCreationPath){.next = plan->first_path, .required_from = consumer, .owner = owner, .site = site};
    plan->first_path = path;
    plan->path_count++;
    *output = path;
    return true;
}

/** Require real provenance before inspecting the value. Only complete ordinary
 * Types prepare a new owner; unknowns remain pending and residual constructors
 * are refused. Normalization never unfolds the required Type's definition.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) forbids guessing unknown Types.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) retains every required definition.
 */
bool static_guard_require_type(StaticGuardPlan *plan, const StaticCreationPath *consumer,
    const StaticTerm *type, SourceSpan site, const StaticCreationPath **output, StaticConstraintOutcome *outcome) {
    if (!static_guard_dependencies(plan)) { return false; }
    if (!output || !outcome || !static_guard_has_path(plan, consumer) ||
        !static_term_has_value(plan->applications->terms, type) ||
        site.source != consumer->owner->environment->definition->definition->source->text || !source_span_valid(site)) {
        return static_guard_fail(plan, STATIC_GUARD_INVALID, (SourceSpan){0}, "Invalid required Type provenance or receiver");
    }
    const StaticTerm *value;
    StaticConstraintOutcome known;
    if (!static_constraint_value(plan->applications->solver, type, &value, &known)) {
        (void)static_guard_dependencies(plan); return false;
    }
    if (known == STATIC_CONSTRAINT_PENDING) { *outcome = known; return true; }
    if (value->kind != STATIC_TERM_NOMINAL || value->category != STATIC_CATEGORY_TYPE) {
        return static_guard_fail(plan, STATIC_GUARD_CONFLICT, site, "Required runtime Type must be a complete ordinary module Type");
    }
    const StaticTermEnvironment *environment;
    if (!static_term_environment(plan->applications->terms, value->as.nominal.origin, value, &environment)) {
        (void)static_guard_dependencies(plan); return false;
    }
    const StaticGuardOwner *owner;
    const StaticCreationPath *path;
    if (!static_guard_owner(plan, environment, &owner) || !requirement(plan, consumer, owner, site, &path)) { return false; }
    StaticGuardClosure closure;
    if (!static_guard_refresh(plan, &closure)) { return false; }
    *output = path;
    *outcome = STATIC_CONSTRAINT_COMPLETE;
    return true;
}
