/** @file
 * @brief Own continuation-copy sources and actual compiler control-map registration.
 */
#ifndef AMA_FOREACH_COPY_TEST_COMMON
#define AMA_FOREACH_COPY_TEST_COMMON
#include "../traversal-origin/common.c"
#include "runtime/program/lists/traversal/copy/copy.h"
#include <stdio.h>

/** Preserve the specific copy-phase call site when the shared assertion fails. */
static void checked_copy(AmaStatus status, const char *path, int line) {
    if (status.code != AMA_OK) { fprintf(stderr, "%s:%d: copy status %u\n", path, line, (unsigned)status.code); }
    checked(status);
}
#define checked(expression) checked_copy((expression), __FILE__, __LINE__)

/** Borrow a live copied root with the same checked nominal storage layout. */
static OriginOwner *copy_value(AmaHandle handle) {
    AmaPoolCell *cell = NULL;
    checked(ama_program_cell(&program, handle, &cell));
    return ama_pool_payload(&owner_type.pool, cell);
}
/** Install exactly the context factory's compiler callbacks on this manual type. */
static void setup_copy(bool local, bool other_source) {
    setup_origins(local, other_source);
    owner_type.controls = &ama_program_foreach_controls;
    owner_type.control_context = &program;
}
/** Publish a real host yield after the test has established its lexical body state. */
static void yield_copy_source(void) {
    AmaPoolCell *cell = NULL;
    checked(ama_program_cell(&program, owners[0], &cell));
    cell->metadata.service.resume = (AmaResumePosition){.valid = true, .selector = 777};
    cell->metadata.service.return_link = (AmaReturnLink){.attached = true, .host = true};
    uint64_t continuation = UINT64_MAX;
    checked(ama_program_yield(&program, &program.current, &continuation));
    assert(continuation == 0 && cell->metadata.service.activity == AMA_INSTANCE_YIELDED);
}
/** Run only the checked resume validator in its required Running owner context;
 * actual saved-body label dispatch is covered by compiled project tests.
 */
static uint64_t inspect_copy_resume(AmaHandle handle) {
    AmaPoolCell *cell = NULL;
    checked(ama_program_cell(&program, handle, &cell));
    AmaHandle previous = program.current;
    AmaInstanceActivity activity = cell->metadata.service.activity;
    program.current = handle;
    cell->metadata.service.activity = AMA_INSTANCE_RUNNING;
    uint64_t repeat = UINT64_MAX;
    checked(ama_program_foreach_resume(&program, handle, &repeat));
    cell->metadata.service.activity = activity;
    program.current = previous;
    return repeat;
}
/** Consume a copied independent root and all its owned descendants, never source roots. */
static void discard_copy(AmaHandle handle) {
    checked(ama_program_foreach_reset(&program, handle));
    checked(ama_object_destroy(&world, handle));
}
/** Discard the captured source invocation before the shared fixture marks its
 * ordinary roots Ready; Ready with a retained resume selector is inconsistent.
 */
static void finish_copy(void) {
    checked(ama_program_foreach_reset(&program, owners[0]));
    AmaPoolCell *cell = NULL;
    checked(ama_program_cell(&program, owners[0], &cell));
    cell->metadata.service.resume = (AmaResumePosition){0};
    cell->metadata.service.activity = AMA_INSTANCE_READY;
    finish_origins();
}
/** Compare identities as values, without depending on C structure padding. */
static bool same_membership(AmaMembership left, AmaMembership right) {
    return ama_handle_equal(left.owner, right.owner) && left.field == right.field
        && left.incarnation == right.incarnation && left.record == right.record && left.generation == right.generation;
}
/** Compare the complete numerical/identity state protected by continuation capture. */
static bool same_cursor(const AmaProgramForeachCursor *left, const AmaProgramForeachCursor *right) {
    return ama_handle_equal(left->owner, right->owner) && left->field == right->field
        && left->incarnation == right->incarnation && left->boundary == right->boundary
        && same_membership(left->current, right->current) && ama_handle_equal(left->target, right->target)
        && left->active == right->active && left->role_active == right->role_active;
}
/** Compare reference/association values, excluding unspecified padding bytes. */
static bool same_binding(const AmaProgramForeachBinding *left, const AmaProgramForeachBinding *right) {
    return ama_handle_equal(left->local_value, right->local_value) && same_membership(left->origin, right->origin)
        && ama_handle_equal(left->installed, right->installed) && left->roles == right->roles
        && left->associated == right->associated;
}
#endif
