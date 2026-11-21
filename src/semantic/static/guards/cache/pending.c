/** @file
 * @brief Suspend cached creation recurrences whose complete comparison keys are not yet known.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) requires unknown
 * repeated keys to stop unresolved source-owner expansion across retained paths.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) bounds
 * operation-local reverse-reachability storage and failure ownership.
 */
#include <stdint.h>
#include <string.h>
#include "view.h"
#include "../internal.h"

/** Without any registered occurrence of the candidate origin, a cached
 * recurrence is impossible. This exact empty-domain check changes no keys.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) compares original
 * module identities rather than names or approximate signatures.
 */
static bool occurs(const StaticGuardPlan *plan, const StaticModule *origin) {
    for (const StaticCreationPath *path = plan->first_path; path; path = path->next) {
        if (path->owner->environment->definition == origin) { return true; }
    }
    return false;
}

/** Each exact validation class enters the queue once. Reversing only real
 * parent edges excludes sibling branches and supplied-Type attribution links.
 * Full keys and unresolved exact owners use the existing validation view.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) retains the lexical
 * creation relation while distinguishing forwarding from construction.
 */
static bool search(StaticGuardPlan *plan, const StaticCreationView *view,
    const StaticCreationPath *parent, const StaticGuardOwner *owner, SourceArena *scratch, bool *pending) {
    if (view->count > SIZE_MAX / sizeof(size_t) || view->count > SIZE_MAX / sizeof(bool)) {
        return static_guard_fail(plan, STATIC_GUARD_OUT_OF_MEMORY, parent->site, "Cached predecessor queue exceeds storage");
    }
    size_t *queue = source_arena_allocate(scratch, view->count * sizeof(*queue));
    bool *seen = source_arena_allocate(scratch, view->count * sizeof(*seen));
    if (!queue || !seen) {
        return static_guard_fail(plan, STATIC_GUARD_OUT_OF_MEMORY, parent->site, "Cached predecessor search allocation failed");
    }
    memset(seen, 0, view->count * sizeof(*seen));
    if (!static_creation_view_path(plan, view, parent, &queue[0])) { return false; }
    seen[queue[0]] = true;
    bool repeated = false;
    size_t count = 1;
    for (size_t front = 0; front < count; front++) {
        size_t current = queue[front];
        const StaticCreationViewNode *node = &view->nodes[current];
        if (node->origin == owner->environment->definition) {
            repeated = true;
            if (node->vertex == NULL) { *pending = true; return true; }
        }
        for (const StaticCreationPath *path = plan->first_path; path; path = path->next) {
            if (!path->parent) { continue; }
            size_t target, source;
            if (!static_creation_view_path(plan, view, path, &target)) { return false; }
            if (target != current) { continue; }
            if (!static_creation_view_path(plan, view, path->parent, &source)) { return false; }
            if (!seen[source]) { seen[source] = true; queue[count++] = source; }
        }
    }
    /* Delay candidate-key normalization until every repeated predecessor is
     * closed. Otherwise a known candidate could intern its environment before
     * a later unknown ancestor correctly suspends entry under
     * [TPL-11](../../../../../spec/language/templates.md#tpl-11). */
    if (repeated) {
        const StaticTermEnvironment *key = NULL;
        StaticConstraintOutcome known;
        if (!static_guard_owner_key(plan, owner, &key, &known)) { return false; }
        if (known == STATIC_CONSTRAINT_PENDING) { *pending = true; }
    }
    return true;
}

/** Actual ancestry is checked directly. Reverse checks supplement an open
 * annotation boundary, including its actual descendants, whose physical chain
 * no longer contains earlier incoming owners. Closed annotation owners have
 * finite canonical identity and can compute independently of pending callers.
 * Successful scratch is always released. Pending never adds a target scope or
 * environment; a successful comparison may intern normalized key metadata.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) suspends before
 * speculative repeated definition expansion, including after cache joins.
 */
bool static_guard_cached_pending(StaticGuardPlan *plan, const StaticCreationPath *parent,
    const StaticGuardOwner *owner, bool *pending) {
    if (*pending || !occurs(plan, owner->environment->definition)) { return true; }
    const StaticCreationPath *annotation = parent;
    while (annotation && !annotation->source_only) { annotation = annotation->parent; }
    if (!annotation) { return true; }
    StaticGuardClosure closure;
    if (!static_guard_refresh(plan, &closure)) { return false; }
    const StaticTermEnvironment *parent_key = NULL;
    StaticConstraintOutcome parent_known;
    if (!static_guard_owner_key(plan, annotation->owner, &parent_key, &parent_known)) { return false; }
    if (parent_known == STATIC_CONSTRAINT_COMPLETE) { return true; }
    SourceArena scratch;
    source_arena_init(&scratch, plan->arena.allocator);
    StaticCreationView view;
    bool success = static_creation_view(plan, &scratch, &view) &&
        search(plan, &view, parent, owner, &scratch, pending);
    source_arena_destroy(&scratch);
    return success;
}
