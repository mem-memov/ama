/** @file
 * @brief Own deep deletion forests and reuse actual typed parameter/native fixtures.
 */
#ifndef AMA_LIST_DELETION_TEST_COMMON
#define AMA_LIST_DELETION_TEST_COMMON
#include "../list-insertion/common.c"
#include "runtime/program/lists/deletion/deletion.h"
#include "runtime/lists/list.h"

/** An owned entry has a separate native child so subtree preflight is observable. */
typedef struct { AmaHandle child; unsigned char payload; } DeletionElement;
static AmaObjectType deletion_type;
static AmaObjectType *deletion_types[] = {&result_type, &owner_type, &integer_type, &deletion_type};
static AmaObjectField deletion_owner_fields[4];

/** Borrow one known public owned entry's child through checked pool storage. */
static AmaHandle deletion_child(AmaHandle handle) {
    AmaPoolCell *cell = NULL;
    AmaStatus status = ama_program_cell(&program, handle, &cell);
    assert(status.code == AMA_OK);
    return ((DeletionElement *)ama_pool_payload(&deletion_type.pool, cell))->child;
}

/** Seed a real empty or two-entry deep forest before disabling element creation/copy.
 * Delete must use neither native initialize nor clone, even if those would fail.
 */
static void setup_deletion(bool seed) {
    setup_insertion();
    static const AmaObjectField child = {AMA_FIELD_OWNED, 1, offsetof(DeletionElement, child)};
    AmaStatus status = ama_object_type_init(&deletion_type, sources.allocator, 4, sizeof(DeletionElement),
        2, UINT64_MAX, &child, 1, offsetof(DeletionElement, payload), (AmaPayloadHooks){0});
    assert(status.code == AMA_OK && owner_type.field_count == 4);
    memcpy(deletion_owner_fields, owner_type.fields, sizeof(deletion_owner_fields));
    deletion_owner_fields[1].type_id = 4;
    owner_type.fields = deletion_owner_fields;
    world.types = deletion_types;
    world.type_count = 4;
    if (seed) {
        AmaListStorage storage = {&world, owners[0], &owner_value(0)->other, 4, true};
        for (size_t index = 0; index < 2; index++) {
            status = ama_list_storage_insert(&storage, index, (AmaHandle){0});
            assert(status.code == AMA_OK);
        }
    }
    result_type.hooks.initialize = insertion_busy_constructor;
    result_type.hooks.clone = insertion_busy_clone;
}

/** Clear any deliberately preserved forest before disposing its extra pool, then
 * use the shared exact native/input/owner teardown and counted allocation check.
 */
static void finish_deletion(void) {
    AmaHandleList *list = &owner_value(0)->other;
    if (list->count != 0) {
        AmaListStorage storage = {&world, owners[0], list, 4, true};
        AmaStatus status = ama_list_storage_remove(&storage, 0, list->count);
        assert(status.code == AMA_OK);
    }
    assert(deletion_type.pool.live_count == 0);
    ama_pool_dispose(&deletion_type.pool);
    finish_insertion();
}
#endif
