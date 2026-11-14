/** @file
 * @brief Bind real lexical cursor catalogs to declared list mutation fixtures.
 */
#ifndef AMA_TRAVERSAL_MUTATION_COMMON
#define AMA_TRAVERSAL_MUTATION_COMMON
#include "../traversal/common.c"
static const size_t cursor_offsets[] = {
    offsetof(TraversalOwner, cursors),
    offsetof(TraversalOwner, cursors) + sizeof(AmaProgramForeachCursor)
};
static const AmaListCursorSet reference_cursors[] = {{cursor_offsets, 2}, {0}, {0}};
static const AmaListCursorSet owned_cursors[] = {{0}, {0}, {cursor_offsets, 2}};

/** Attach the exact fixed catalog before starting either actual lexical scan. */
static void setup_correction(size_t count) {
    setup_traversal();
    owner_type.list_cursors = reference_cursors;
    for (size_t index = 0; index < count; index++) { append_reference(owners[1]); }
}
/** Activate a real cursor and consume exactly boundary eligible candidates. */
static AmaProgramForeachPlan advance_correction(size_t site, size_t boundary) {
    AmaProgramForeachPlan plan = traversal_plan(site, false);
    checked(ama_program_foreach_enter(&program, owners[0], &plan));
    for (size_t index = 0; index < boundary; index++) {
        bool selected = false;
        checked(ama_program_foreach_next(&program, owners[0], &plan, &selected));
        assert(selected);
    }
    return plan;
}
/** Obtain a guarded primitive's actual declared reference-list storage. */
static AmaListStorage correction_storage(void) {
    return (AmaListStorage){&world, owners[0], &owner_value(0)->items, 1, false};
}
/** Abandon both fixed lexical roles before pool cleanup, without target effects. */
static void finish_correction(void) {
    for (size_t site = 0; site < 2; site++) {
        AmaProgramForeachPlan plan = traversal_plan(site, false);
        checked(ama_program_foreach_leave(&program, owners[0], &plan, true));
    }
    finish_traversal();
}
#endif
