/** @file
 * @brief Rebuild closed substituted values bottom-up with shared iterative frames.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) preserves finite shared terms.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns partial reconstruction.
 */
#include <stdint.h>
#include "internal.h"

/** Frames memoize resolved input identities; a child completes before its parent
 * capture is published. [TPL-10](../../../../../spec/language/templates.md#tpl-10).
 */
typedef struct ConstraintMaterialization {
    struct ConstraintMaterialization *next;
    struct ConstraintMaterialization *parent;
    const StaticTerm *term;
    const StaticTerm *value;
    const StaticTermCapture *cursor;
    StaticTermCapture *captures;
    size_t count;
} ConstraintMaterialization;

/** Existing ground terms contain no variable anywhere, hence substitution cannot
 * change their canonical identity. Shared symbolic vertices get one frame.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) avoids equivalent repeated work.
 */
static ConstraintMaterialization *prepare(StaticConstraintSolver *solver, SourceArena *scratch, const StaticTerm *term,
    ConstraintMaterialization *parent, ConstraintMaterialization **memo) {
    term = static_constraint_root(solver, term);
    for (ConstraintMaterialization *item = *memo; item; item = item->next) {
        if (item->term == term) { return item; }
    }
    ConstraintMaterialization *item = static_constraint_scratch(solver, scratch, sizeof(*item));
    if (item == NULL) { return NULL; }
    *item = (ConstraintMaterialization){.term = term, .parent = parent};
    if (term->ground) { item->value = term; }
    else {
        size_t count = term->as.nominal.capture_count;
        if (count > SIZE_MAX / sizeof(*item->captures)) {
            (void)static_constraint_failure(solver, STATIC_CONSTRAINT_OUT_OF_MEMORY, term->site,
                "Resolved capture storage exceeds representable size", term, NULL);
            return NULL;
        }
        if (count != 0) {
            item->captures = static_constraint_scratch(solver, scratch, count * sizeof(*item->captures));
            if (item->captures == NULL) { return NULL; }
        }
        item->cursor = term->as.nominal.first_capture;
    }
    item->next = *memo;
    *memo = item;
    return item;
}

/** Substitute capture values only; no source body/layout or runtime state enters
 * reconstruction. Checked source descriptions transfer to equivalent results.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) preserves static metadata.
 */
static bool complete(StaticConstraintSolver *solver, ConstraintMaterialization *item) {
    const StaticTerm *value;
    if (!static_term_nominal(solver->terms, item->term->as.nominal.origin, item->captures,
        item->count, item->term->site, &value)) {
        StaticConstraintStatus status = solver->terms->status == STATIC_TERM_OUT_OF_MEMORY ?
            STATIC_CONSTRAINT_OUT_OF_MEMORY : STATIC_CONSTRAINT_INVALID;
        return static_constraint_failure(solver, status, item->term->site,
            "Resolved static value could not be constructed", item->term, NULL);
    }
    StaticCategory category;
    const StaticSignature *signature;
    static_constraint_description(solver, item->term, &category, &signature);
    if (value != item->term && !static_constraint_describe(solver, value, category, signature, item->term->site)) { return false; }
    item->value = value;
    return true;
}

/** The current frame waits at its original capture until the child's memoized
 * result exists; popping that child advances the parent exactly once.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) requires finite structural progress.
 */
bool static_constraint_materialize(StaticConstraintSolver *solver, const StaticTerm *term, const StaticTerm **output) {
    SourceArena scratch;
    source_arena_init(&scratch, solver->arena.allocator);
    ConstraintMaterialization *memo = NULL;
    ConstraintMaterialization *root = prepare(solver, &scratch, term, NULL, &memo);
    bool success = root != NULL;
    ConstraintMaterialization *current = root;
    while (success && current != NULL) {
        if (current->value != NULL) { current = current->parent; continue; }
        if (current->cursor == NULL) {
            if (!complete(solver, current)) { success = false; break; }
            current = current->parent;
            continue;
        }
        ConstraintMaterialization *child = prepare(solver, &scratch, current->cursor->value, current, &memo);
        if (child == NULL) { success = false; break; }
        if (child->value == NULL) { current = child; continue; }
        current->captures[current->count++] = (StaticTermCapture){
            .formal = current->cursor->formal, .value = child->value};
        current->cursor = current->cursor->next;
    }
    const StaticTerm *value = success ? root->value : NULL;
    source_arena_destroy(&scratch);
    if (success) { *output = value; }
    return success;
}
