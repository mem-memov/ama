/** @file
 * @brief Own checked membership origins, shared bindings and independent references.
 */
#ifndef AMA_TRAVERSAL_ORIGIN_COMMON
#define AMA_TRAVERSAL_ORIGIN_COMMON
#include "../../source/common.c"
#include "runtime/program/lists/traversal/traversal.h"
#include "runtime/program/lists/insertion/insertion.h"
#include "runtime/program/calls/call.h"
#include "runtime/lists/list.h"
#include <stddef.h>

/** Only the five described fields contain persistent Ama data; remaining cells
 * are compiler-visible lexical/operation state outside native payload copying.
 */
typedef struct {
    AmaHandleList items;
    AmaHandle reused, independent;
    AmaHandleList other, owned;
    unsigned char payload;
    AmaProgramForeachCursor cursors[2];
    AmaProgramForeachBinding binding;
    AmaProgramFrame frame;
    AmaProgramListService service;
    AmaProgramListMethod method;
} OriginOwner;
static AmaObjectType owner_type;
static AmaObjectType *types[] = {&owner_type};
static AmaWorld world;
static AmaProgram program;
static AmaProgramType program_type;
static AmaProgramForeachScope scopes[2];
static AmaHandle owners[3];
static size_t conversions;
static const size_t cursor_offsets[] = {offsetof(OriginOwner, cursors),
    offsetof(OriginOwner, cursors) + sizeof(AmaProgramForeachCursor)};
static const AmaListCursorSet same_source[] = {{cursor_offsets, 2}, {0}, {0}, {0}, {0}};
static const AmaListCursorSet separate_sources[] = {{cursor_offsets, 1}, {0}, {0}, {cursor_offsets + 1, 1}, {0}};

/** Require successful fixture operations before checking observable contracts. */
static void checked(AmaStatus status) { assert(status.code == AMA_OK); }
/** Borrow a stable owner only while it remains a live fixture root. */
static OriginOwner *owner_value(size_t index) {
    AmaPoolCell *cell = NULL;
    checked(ama_program_cell(&program, owners[index], &cell));
    return ama_pool_payload(&owner_type.pool, cell);
}
/** An observable declared protocol proves origin refusal precedes conversion. */
static AmaStatus origin_condition(const void *payload, bool *truth) {
    (void)payload;
    conversions++;
    *truth = true;
    return ama_status_success();
}
/** The same observation for an index operand, without assuming a payload layout. */
static AmaStatus origin_index(const void *payload, size_t *position) {
    (void)payload;
    conversions++;
    *position = 0;
    return ama_status_success();
}
/** Seed checked reference occurrences before activating any lexical scope. */
static void append_origin_reference(AmaHandleList *list, AmaHandle target) {
    AmaMembership membership;
    checked(ama_membership_insert(list, list->count, target, &membership));
}
/** Install two nested roles over actual declared fields and three independent roots. */
static void setup_origins(bool local, bool other_source) {
    setup_test();
    static const AmaObjectField fields[] = {
        {AMA_FIELD_REFERENCE_LIST, 1, offsetof(OriginOwner, items)},
        {AMA_FIELD_REFERENCE, 1, offsetof(OriginOwner, reused)},
        {AMA_FIELD_REFERENCE, 1, offsetof(OriginOwner, independent)},
        {AMA_FIELD_REFERENCE_LIST, 1, offsetof(OriginOwner, other)},
        {AMA_FIELD_OWNING_LIST, 1, offsetof(OriginOwner, owned)}
    };
    static const AmaProgramField declarations[] = {
        {.role = AMA_PROGRAM_STATE}, {.role = AMA_PROGRAM_STATE}, {.role = AMA_PROGRAM_STATE},
        {.role = AMA_PROGRAM_STATE}, {.role = AMA_PROGRAM_STATE}
    };
    checked(ama_object_type_init(&owner_type, sources.allocator, 1, sizeof(OriginOwner),
        2, UINT64_MAX, fields, 5, offsetof(OriginOwner, payload), (AmaPayloadHooks){0}));
    owner_type.list_cursors = other_source ? separate_sources : same_source;
    for (size_t index = 0; index < 2; index++) {
        scopes[index] = (AmaProgramForeachScope){.parent = index == 0 ? SIZE_MAX : 0,
            .header = 100 + index * 100, .repeat = 101 + index * 100,
            .plan = {.field = index != 0 && other_source ? 3 : 0,
                .cursor_offset = cursor_offsets[index], .binding_offset = offsetof(OriginOwner, binding),
                .value_offset = local ? offsetof(OriginOwner, binding) + offsetof(AmaProgramForeachBinding, local_value)
                    : offsetof(OriginOwner, reused), .element_type = 1, .local = local}};
    }
    checked(ama_program_foreach_schema(sizeof(OriginOwner), fields, 5, scopes, 2));
    program_type = (AmaProgramType){.object = &owner_type, .fields = declarations, .field_count = 5,
        .condition = origin_condition, .index = origin_index, .foreach_scopes = scopes, .foreach_count = 2};
    world = (AmaWorld){.types = types, .type_count = 1, .allocator = sources.allocator};
    program = (AmaProgram){.world = &world, .types = &program_type, .type_count = 1};
    conversions = 0;
    for (size_t index = 0; index < 3; index++) { checked(ama_object_create(&world, 1, NULL, &owners[index])); }
    program.current = owners[0];
    AmaPoolCell *cell = NULL;
    checked(ama_program_cell(&program, owners[0], &cell));
    cell->metadata.service.activity = AMA_INSTANCE_RUNNING;
    append_origin_reference(&owner_value(0)->items, owners[1]);
    append_origin_reference(&owner_value(0)->items, owners[2]);
    append_origin_reference(&owner_value(0)->other, owners[2]);
}
/** Advance an actual lexical site by its required number of eligible candidates. */
static void select_origin(size_t site, size_t count) {
    checked(ama_program_foreach_enter(&program, owners[0], &scopes[site].plan));
    for (size_t index = 0; index < count; index++) {
        bool selected;
        checked(ama_program_foreach_next(&program, owners[0], &scopes[site].plan, &selected));
        assert(selected);
    }
}
/** Remove a real occurrence through the corrected structural publication path. */
static void remove_origin(size_t position) {
    AmaListStorage storage = {&world, owners[0], &owner_value(0)->items, 1, false};
    checked(ama_list_storage_remove(&storage, position, 1));
}
/** Obtain a standard resolved access, including binding location when applicable. */
static AmaProgramAccess origin_access(void) {
    return (AmaProgramAccess){.field = 1, .binding = &scopes[0].plan};
}
/** Release every actual lexical/operation role before independent root destruction. */
static void finish_origins(void) {
    checked(ama_program_foreach_reset(&program, owners[0]));
    OriginOwner *owner = owner_value(0);
    assert(owner->binding.roles == 0 && !owner->service.active && !owner->method.active && !owner->frame.active);
    for (size_t index = 0; index < 3; index++) {
        AmaPoolCell *cell = NULL;
        checked(ama_program_cell(&program, owners[index], &cell));
        cell->metadata.service.activity = AMA_INSTANCE_READY;
    }
    for (size_t index = 3; index != 0; index--) { checked(ama_object_destroy(&world, owners[index - 1])); }
    ama_pool_dispose(&owner_type.pool);
    destroy_test();
}
#endif
