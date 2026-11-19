/** @file
 * @brief Normalize ordinary and static producer owners without conflating their categories.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) compares complete keys.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) retains all producer inputs.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns temporary storage.
 */
#include <stdint.h>
#include "../internal.h"

/** Solver closure, rather than groundness, authorizes canonical owner sharing.
 * Normalize every producer input even if another remains pending, so known
 * constraint conflicts are not hidden. Scratch expires before returning.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) separates solving from admission.
 */
bool static_guard_owner_key(StaticGuardPlan *plan, const StaticGuardOwner *owner,
    const StaticTermEnvironment **output, StaticConstraintOutcome *outcome) {
    StaticTermStore *terms = plan->applications->terms;
    StaticConstraintSolver *solver = plan->applications->solver;
    if (owner->value != NULL) {
        const StaticTerm *value;
        if (!static_constraint_value(solver, owner->value, &value, outcome)) {
            (void)static_guard_dependencies(plan); return false;
        }
        if (*outcome == STATIC_CONSTRAINT_PENDING) { return true; }
        if (!static_term_environment(terms, owner->environment->definition, value, output)) {
            (void)static_guard_dependencies(plan); return false;
        }
        return true;
    }
    size_t count = owner->key->definition->parameter_count;
    if (count > SIZE_MAX / sizeof(StaticTermCapture)) {
        return static_guard_fail(plan, STATIC_GUARD_OUT_OF_MEMORY, (SourceSpan){0},
            "Producer normalization input map exceeds representable storage");
    }
    SourceArena scratch;
    source_arena_init(&scratch, plan->arena.allocator);
    StaticTermCapture *inputs = count ? source_arena_allocate(&scratch, count * sizeof(*inputs)) : NULL;
    if (count && inputs == NULL) {
        source_arena_destroy(&scratch);
        return static_guard_fail(plan, STATIC_GUARD_OUT_OF_MEMORY, (SourceSpan){0},
            "Producer normalization could not allocate input map");
    }
    *outcome = STATIC_CONSTRAINT_COMPLETE;
    bool success = true;
    size_t index = 0;
    for (const StaticTermCapture *input = owner->key->producer_inputs; input; input = input->next) {
        StaticConstraintOutcome current;
        inputs[index].formal = input->formal;
        if (!static_constraint_value(solver, input->value, &inputs[index].value, &current)) {
            (void)static_guard_dependencies(plan); success = false; break;
        }
        if (current == STATIC_CONSTRAINT_PENDING) { *outcome = current; }
        index++;
    }
    if (success && *outcome == STATIC_CONSTRAINT_COMPLETE &&
        !static_term_producer_environment(terms, owner->key->definition, inputs, count, output)) {
        (void)static_guard_dependencies(plan); success = false;
    }
    source_arena_destroy(&scratch);
    return success;
}
