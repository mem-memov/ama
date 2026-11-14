/** @file
 * @brief Own real declared lists, reused references and fixed lexical scan cells.
 */
#ifndef AMA_FOREACH_TEST_COMMON
#define AMA_FOREACH_TEST_COMMON
#include "../../source/common.c"
#include "runtime/program/lists/traversal/traversal.h"
#include "runtime/program/calls/call.h"
#include "runtime/lists/list.h"
#include <stddef.h>

/** Declared data and lexical control slots have separate explicit locations. */
typedef struct {
    AmaHandleList items;
    AmaHandle reused;
    AmaHandleList owned;
    unsigned char payload;
    AmaProgramForeachCursor cursors[2];
    AmaProgramForeachBinding binding;
    AmaProgramFrame call_frame;
} TraversalOwner;
static AmaObjectType owner_type;
static AmaObjectType *types[] = {&owner_type};
static AmaWorld world;
static AmaProgram program;
static AmaHandle owners[2];

/** Require successful product operations before making semantic assertions. */
static void checked(AmaStatus status) { assert(status.code == AMA_OK); }
/** Borrow a known live root only for the duration of one test operation. */
static TraversalOwner *owner_value(size_t index) {
    AmaPoolCell *cell = NULL;
    checked(ama_program_cell(&program, owners[index], &cell));
    return ama_pool_payload(&owner_type.pool, cell);
}
/** Bind both nested sites to the same cell; reused persistent storage is separate. */
static AmaProgramForeachPlan traversal_plan(size_t index, bool local) {
    return (AmaProgramForeachPlan){.field = 0,
        .cursor_offset = offsetof(TraversalOwner, cursors) + index * sizeof(AmaProgramForeachCursor),
        .binding_offset = offsetof(TraversalOwner, binding), .element_type = 1, .local = local,
        .value_offset = local ? offsetof(TraversalOwner, binding) + offsetof(AmaProgramForeachBinding, local_value)
            : offsetof(TraversalOwner, reused)};
}
/** Construct two genuine owners and make only the initiating root Running. */
static void setup_traversal(void) {
    setup_test();
    static const AmaObjectField fields[] = {
        {AMA_FIELD_REFERENCE_LIST, 1, offsetof(TraversalOwner, items)},
        {AMA_FIELD_REFERENCE, 1, offsetof(TraversalOwner, reused)},
        {AMA_FIELD_OWNING_LIST, 1, offsetof(TraversalOwner, owned)}
    };
    checked(ama_object_type_init(&owner_type, sources.allocator, 1, sizeof(TraversalOwner),
        2, UINT64_MAX, fields, 3, offsetof(TraversalOwner, payload), (AmaPayloadHooks){0}));
    world = (AmaWorld){.types = types, .type_count = 1, .allocator = sources.allocator};
    program = (AmaProgram){.world = &world};
    for (size_t index = 0; index < 2; index++) { checked(ama_object_create(&world, 1, NULL, &owners[index])); }
    program.current = owners[0];
    AmaPoolCell *cell = NULL;
    checked(ama_program_cell(&program, owners[0], &cell));
    cell->metadata.service.activity = AMA_INSTANCE_RUNNING;
    owner_value(0)->reused = owners[1];
}
/** Append reference data without acquiring or changing its target state. */
static void append_reference(AmaHandle target) {
    AmaHandleList *list = &owner_value(0)->items;
    AmaMembership identity;
    checked(ama_membership_insert(list, list->count, target, &identity));
}
/** All roles and short acquisitions must be gone before destroying real roots. */
static void finish_traversal(void) {
    TraversalOwner *value = owner_value(0);
    assert(value->items.acquisition == NULL && !value->cursors[0].active
        && !value->cursors[1].active && value->binding.roles == 0);
    AmaPoolCell *cell = NULL;
    checked(ama_program_cell(&program, owners[0], &cell));
    cell->metadata.service.activity = AMA_INSTANCE_READY;
    for (size_t index = 2; index != 0; index--) { checked(ama_object_destroy(&world, owners[index - 1])); }
    ama_pool_dispose(&owner_type.pool);
    destroy_test();
}
#endif
