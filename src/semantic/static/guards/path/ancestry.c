/** @file
 * @brief Close exact nominal ancestry cycles or suspend comparisons lacking complete keys.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) rejects different-key recurrence,
 * while an unknown repeated key must wait before another definition expands.
 * [COMP-04](../../../../../spec/implementation/compiler.md#comp-04) retains both keys and uses.
 */
#include "../internal.h"

/** Retain borrowed canonical values and original uses before sticky refusal.
 * Reporting allocates nothing; all source/term owners outlive the guard.
 * [COMP-04](../../../../../spec/implementation/compiler.md#comp-04) preserves diagnostic provenance.
 */
static bool reject(StaticGuardPlan *plan, const StaticCreationPath *parent,
    const StaticCreationPath *ancestor, const StaticTerm *earlier,
    const StaticTerm *target, SourceSpan site) {
    const StaticModule *origin = ancestor->owner->environment->definition;
    plan->rejected_path = parent;
    plan->repeated_ancestor = ancestor;
    plan->rejected_origin = origin;
    plan->rejected_site = site;
    plan->earlier_creation_value = earlier;
    plan->rejected_creation_value = target;
    (void)static_guard_fail(plan, STATIC_GUARD_CONFLICT, site,
        "Different complete specializations of one template on a creation path");
    const ProjectModule *definition = origin->definition;
    plan->related[0] = (DiagnosticRelated){.location = {.span = {definition->source->text, 0, 0}},
        .role = "repeated source template", .name = definition->source->logical_path};
    plan->related[1] = (DiagnosticRelated){.location = {.span = ancestor->site},
        .role = "earlier specialization on this path", .name = definition->source->logical_path};
    plan->error.related = plan->related;
    plan->error.related_count = 2;
    return false;
}

/** Source identity only locates comparisons; it never proves key equality.
 * Each complete normalized term is interned by T176, so pointer equality then
 * means full nominal equality, including unused bindings and captures. Any
 * unresolved comparison suspends entry, even when another ancestor is equal.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) forbids speculative repeated expansion.
 */
bool static_guard_ancestry(StaticGuardPlan *plan, const StaticCreationPath *parent,
    const StaticTerm *value, SourceSpan site, bool *pending) {
    *pending = false;
    const StaticModule *origin = value->as.nominal.origin;
    StaticTermEnvironment environment = {.definition = origin, .owner_type = value};
    StaticGuardOwner owner = {.environment = &environment, .key = &environment, .value = value};
    return static_guard_owner_ancestry(plan, parent, &owner, site, pending);
}

/** Compare both ordinary and compiler-only producer keys through the same
 * normalization boundary; no unknown repeated key permits another scope.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) requires complete comparisons.
 */
bool static_guard_owner_ancestry(StaticGuardPlan *plan, const StaticCreationPath *parent,
    const StaticGuardOwner *owner, SourceSpan site, bool *pending) {
    *pending = false;
    const StaticModule *origin = owner->environment->definition;
    if (origin->parameter_count == 0) { return true; }
    for (const StaticCreationPath *ancestor = parent; ancestor; ancestor = ancestor->parent) {
        if (ancestor->owner->environment->definition != origin) { continue; }
        const StaticTermEnvironment *earlier = NULL, *target = NULL;
        StaticConstraintOutcome left, right;
        if (!static_guard_owner_key(plan, ancestor->owner, &earlier, &left)) { return false; }
        if (left == STATIC_CONSTRAINT_PENDING) { *pending = true; continue; }
        if (!static_guard_owner_key(plan, owner, &target, &right)) { return false; }
        if (right == STATIC_CONSTRAINT_PENDING) {
            *pending = true;
        } else if (earlier != target) {
            plan->earlier_creation_key = earlier;
            plan->rejected_creation_key = target;
            return reject(plan, parent, ancestor, earlier->owner_type, target->owner_type, site);
        }
    }
    return true;
}
