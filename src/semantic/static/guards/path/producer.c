/** @file
 * @brief Check actual static producer ancestry before exposing a new symbolic producer scope.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) suspends unknown repeats.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) separates producers from values.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns partial preparation.
 */
#include <stdint.h>
#include "../internal.h"

/** Exact source occurrences determine the producer and lexical ownership;
 * matching names or a caller-supplied nominal cannot select another origin.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) retains actual provenance.
 */
static const StaticModule *origin(StaticGuardPlan *plan, const StaticCreationPath *parent,
    const AstTypeExpression *expression) {
    if (!static_guard_has_path(plan, parent) || expression == NULL) { return NULL; }
    for (const StaticOccurrence *site = parent->owner->environment->definition->first_occurrence;
        site; site = site->next) {
        if (site->expression == expression && site->receiver == NULL && site->origin != NULL) {
            const StaticModule *definition = static_module_find(plan->applications->terms->inventory, site->origin);
            if (definition && definition->definition->syntax.sections.kind == MODULE_STATIC) { return definition; }
        }
    }
    return NULL;
}

/** A temporary linked map gives the comparison operation a candidate owner
 * without interning a new scope. Caller arrays are copied, never relinked.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) forbids speculative repeated expansion.
 */
bool static_creation_producer_enter(StaticGuardPlan *plan, const StaticCreationPath *parent,
    const AstTypeExpression *expression, const StaticTermCapture *inputs, size_t count,
    const StaticCreationPath **output, StaticCreationOutcome *outcome) {
    if (!static_guard_dependencies(plan)) { return false; }
    const StaticModule *definition = origin(plan, parent, expression);
    if (output == NULL || outcome == NULL || definition == NULL ||
        !static_term_producer_inputs_valid(plan->applications->terms, definition, inputs, count)) {
        return static_guard_fail(plan, STATIC_GUARD_INVALID, (SourceSpan){0}, "Invalid lexical static producer request");
    }
    if (count > SIZE_MAX / sizeof(StaticTermCapture)) {
        return static_guard_fail(plan, STATIC_GUARD_OUT_OF_MEMORY, expression->span, "Producer comparison map exceeds storage");
    }
    SourceArena scratch;
    source_arena_init(&scratch, plan->arena.allocator);
    StaticTermCapture *map = count ? source_arena_allocate(&scratch, count * sizeof(*map)) : NULL;
    if (count && map == NULL) {
        source_arena_destroy(&scratch);
        return static_guard_fail(plan, STATIC_GUARD_OUT_OF_MEMORY, expression->span, "Producer comparison map allocation failed");
    }
    for (size_t index = 0; index < count; index++) {
        map[index] = inputs[index];
        map[index].next = index + 1 < count ? &map[index + 1] : NULL;
    }
    StaticTermEnvironment candidate = {.definition = definition, .producer_inputs = map, .producer_applied = true};
    StaticGuardOwner owner = {.environment = &candidate, .key = &candidate};
    bool pending;
    bool success = static_guard_owner_ancestry(plan, parent, &owner, expression->span, &pending);
    source_arena_destroy(&scratch);
    if (!success) { return false; }
    if (pending) { *outcome = STATIC_CREATION_PENDING; return true; }
    const StaticTermEnvironment *environment;
    if (!static_term_producer_environment(plan->applications->terms, definition, inputs, count, &environment)) {
        (void)static_guard_dependencies(plan); return false;
    }
    const StaticGuardOwner *published;
    const StaticCreationPath *path;
    if (!static_guard_owner(plan, environment, &published) ||
        !static_guard_path(plan, parent, published, NULL, expression->span, &path)) { return false; }
    StaticGuardClosure closure;
    if (!static_guard_refresh(plan, &closure)) { return false; }
    *output = path;
    *outcome = STATIC_CREATION_ENTERED;
    return true;
}
