/** @file
 * @brief Intern exact lexical owner shapes while retaining unresolved captured variables.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) retains symbolic environments.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns failed preparation.
 */
#include <stdint.h>
#include "../internal.h"

/** Every original formal is bound to its existing scoped variable; this merely
 * describes the owner shape and does not turn residual metadata into new holes.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves parameter scope.
 */
static bool generic(StaticGuardPlan *plan, const StaticTermEnvironment *environment, const StaticTerm **output) {
    size_t count = environment->definition->parameter_count;
    if (count > SIZE_MAX / sizeof(StaticTermCapture)) {
        return static_guard_fail(plan, STATIC_GUARD_OUT_OF_MEMORY, (SourceSpan){0},
            "Symbolic guard owner exceeds representable capture storage");
    }
    StaticTermCapture *captures = count ? static_guard_allocate(plan, count * sizeof(*captures)) : NULL;
    if (count != 0 && captures == NULL) { return false; }
    StaticTermStore *terms = plan->applications->terms;
    for (size_t ordinal = 0; ordinal < count; ordinal++) {
        captures[ordinal].formal = static_symbol_at(environment->definition, STATIC_PARAMETER, ordinal);
        if (!static_term_symbol(terms, environment, captures[ordinal].formal, &captures[ordinal].value)) {
            (void)static_guard_dependencies(plan); return false;
        }
    }
    if (!static_term_nominal(terms, environment->definition, captures, count, (SourceSpan){0}, output)) {
        (void)static_guard_dependencies(plan); return false;
    }
    return true;
}

/** Exact environment membership precedes dereference, and full original maps
 * preserve unused formals. Applied owners retain their original symbolic key.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves source ownership.
 */
bool static_guard_owner(StaticGuardPlan *plan, const StaticTermEnvironment *environment, const StaticGuardOwner **output) {
    if (!static_guard_dependencies(plan)) { return false; }
    if (output == NULL || !static_term_has_environment(plan->applications->terms, environment)) {
        return static_guard_fail(plan, STATIC_GUARD_INVALID, (SourceSpan){0}, "Invalid lexical guard owner");
    }
    for (const StaticGuardOwner *owner = plan->first_owner; owner; owner = owner->next) {
        if (owner->environment == environment) { *output = owner; return true; }
    }
    if (plan->owner_count == SIZE_MAX) {
        return static_guard_fail(plan, STATIC_GUARD_OUT_OF_MEMORY, (SourceSpan){0},
            "Static guard owner count exceeds representable storage");
    }
    const StaticTerm *value = environment->owner_type;
    if (value == NULL && !generic(plan, environment, &value)) { return false; }
    StaticGuardOwner *owner = static_guard_allocate(plan, sizeof(*owner));
    if (owner == NULL) { return false; }
    *owner = (StaticGuardOwner){.environment = environment, .value = value, .next = plan->first_owner};
    plan->first_owner = owner;
    plan->owner_count++;
    *output = owner;
    return true;
}
