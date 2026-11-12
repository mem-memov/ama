/**
 * @file
 * @brief Test private forest staging, exact fixups, commit ownership and cleanup failures.
 */
#include "../transfers/common.c"
#include "runtime/program/transfers/list/internal.h"

static AmaObjectOperation operation;
static AmaProgram program;
static AmaHandle source, destination;
static AmaProgramValue source_value, destination_value;

/** Build the real cross-element forest and resolve borrowed list locations. */
static void setup_transfer(void) {
    setup_objects();
    make_source(&source, &destination);
    program = (AmaProgram){.world = &world};
    const AmaProgramAccess access = {.field = 2};
    source_value = destination_value = (AmaProgramValue){0};
    AmaStatus status = ama_program_select(&program, source, &access, false, &source_value);
    assert(status.code == AMA_OK);
    status = ama_program_select(&program, destination, &access, false, &destination_value);
    assert(status.code == AMA_OK);
}

/** Own scratch/source protections through complete staging or rollback. */
static void begin_transfer(void) {
    operation = (AmaObjectOperation){.world = &world};
    source_arena_init(&operation.arena, world.allocator);
    world.operation_active = true;
}

/** Restore protected sources only after staged resources have an owner. */
static void end_transfer(void) {
    ama_object_unprotect(&operation);
    source_arena_destroy(&operation.arena);
    world.operation_active = false;
}

/** Finalize real public owners; no staged resource may survive this boundary. */
static void finish_transfer(void) {
    AmaStatus status = ama_object_destroy(&world, destination);
    assert(status.code == AMA_OK);
    status = ama_object_destroy(&world, source);
    assert(status.code == AMA_OK && constructed == destroyed);
    destroy_objects();
}

/** No-op/shape validation precedes allocations or native side effects. */
static void it_checks_aliases_shapes_and_continuation_self_copy_before_allocation(void) {
    setup_transfer();
    begin_transfer();
    bool no_op = false;
    size_t before = allocation_count;
    AmaStatus status = ama_transfer_list_validate(&source_value, &source_value, &operation, &no_op);
    assert(status.code == AMA_OK && no_op && allocation_count == before);
    source_value.continuation = true;
    status = ama_transfer_list_validate(&source_value, &source_value, &operation, &no_op);
    assert(status.code == AMA_INVALID_STATE && !no_op && allocation_count == before);
    source_value.continuation = false;
    AmaProgramValue invalid = source_value;
    invalid.type_id = 1;
    status = ama_transfer_list_validate(&destination_value, &invalid, &operation, &no_op);
    assert(status.code == AMA_TYPE_MISMATCH && !no_op && allocation_count == before);
    invalid = destination_value;
    invalid.list = NULL;
    status = ama_transfer_list_validate(&invalid, &source_value, &operation, &no_op);
    assert(status.code == AMA_INVALID_STATE && cloned == 0 && allocation_count == before);
    end_transfer();
    finish_transfer();
}

/** Cross-element references are fixed only after every private identity exists. */
static void it_fixes_one_complete_private_forest_before_consuming_staging_at_commit(void) {
    setup_transfer();
    begin_transfer();
    bool no_op;
    AmaStatus status = ama_transfer_list_validate(&destination_value, &source_value, &operation, &no_op);
    assert(status.code == AMA_OK && !no_op);
    AmaHandleList prepared = {0};
    status = ama_transfer_list_prepare(&program, &destination_value, &source_value, NULL, &operation, &prepared);
    assert(status.code == AMA_OK && prepared.count == 2 && destination_value.list->count == 0);
    AmaPoolCell *first = NULL, *second = NULL;
    status = ama_pool_lookup(&root_type.pool, prepared.items[0], true, &first);
    assert(status.code == AMA_OK && !first->metadata.identity.live);
    status = ama_pool_lookup(&root_type.pool, prepared.items[1], true, &second);
    assert(status.code == AMA_OK && !second->metadata.identity.live);
    RootValue *one = ama_pool_payload(&root_type.pool, first);
    RootValue *two = ama_pool_payload(&root_type.pool, second);
    assert(ama_handle_equal(one->self, source_value.list->items[1]));
    ama_transfer_list_fixup(&operation, true);
    assert(ama_handle_equal(one->self, prepared.items[1]) && ama_handle_equal(two->self, prepared.items[0]));
    assert(ama_handle_equal(one->selected, two->left));
    status = ama_transfer_list_commit(&program, &destination_value, &operation, &prepared);
    assert(status.code == AMA_OK && prepared.items == NULL && prepared.count == 0);
    assert(destination_value.list->count == 2 && first->metadata.identity.live && second->metadata.identity.live);
    assert(ama_handle_equal(first->owner, destination) && ama_handle_equal(second->owner, destination));
    AmaPoolCell *parent = NULL;
    status = ama_program_cell(&program, destination, &parent);
    assert(status.code == AMA_OK && parent->metadata.lifetime.live_children == 4);
    end_transfer();
    finish_transfer();
}

/** A precommit identity failure keeps staging owned; irreversible cleanup failure does too. */
static void it_retains_staging_for_cleanup_on_parent_failure_and_destructive_failure(void) {
    for (size_t destructive = 0; destructive < 2; destructive++) {
        setup_transfer();
        if (destructive) {
            const AmaProgramAccess access = {.field = 2};
            AmaStatus status = ama_program_transfer(&program, destination, &access, &source_value);
            assert(status.code == AMA_OK && destination_value.list->count == 2);
        }
        begin_transfer();
        bool no_op;
        AmaStatus status = ama_transfer_list_validate(&destination_value, &source_value, &operation, &no_op);
        assert(status.code == AMA_OK && !no_op);
        AmaHandleList prepared = {0};
        status = ama_transfer_list_prepare(&program, &destination_value, &source_value, NULL, &operation, &prepared);
        assert(status.code == AMA_OK && prepared.count == 2);
        ama_transfer_list_fixup(&operation, true);
        size_t before = destroyed;
        AmaProgramValue invalid = destination_value;
        if (destructive) { failed_destructor = before + 1; }
        else { invalid.container.generation = 99; }
        status = ama_transfer_list_commit(&program, &invalid, &operation, &prepared);
        assert(status.code == (destructive ? AMA_IO_FAILURE : AMA_STALE_REFERENCE));
        assert(prepared.count == 2 && prepared.items != NULL && destination_value.list->count == 0);
        assert(destroyed == before + (destructive ? 6 : 0));
        status = ama_transfer_list_discard(&program, &prepared, true, status);
        assert(status.code == (destructive ? AMA_IO_FAILURE : AMA_STALE_REFERENCE));
        assert(prepared.count == 0 && prepared.items == NULL && destroyed == before + (destructive ? 12 : 6));
        assert(world.failed == (destructive != 0));
        end_transfer();
        finish_transfer();
    }
}

/** Copying a reference list neither acquires nor destroys its targets. */
static void it_copies_reference_handles_including_busy_stale_and_empty_targets(void) {
    setup_transfer();
    RootValue *owner = object_payload(source);
    owner->references.items = test_allocate(NULL, 3 * sizeof(AmaHandle));
    assert(owner->references.items != NULL);
    owner->references.count = owner->references.capacity = 3;
    owner->references.items[0] = owner->left;
    owner->references.items[1] = (AmaHandle){.type_id = 1, .slot_id = 999, .generation = 1};
    owner->references.items[2] = (AmaHandle){0};
    AmaPoolCell *busy = NULL;
    AmaStatus status = ama_pool_lookup(&leaf_type.pool, owner->left, false, &busy);
    assert(status.code == AMA_OK);
    busy->metadata.service.activity = AMA_INSTANCE_RUNNING;
    const AmaProgramAccess access = {.field = 5};
    status = ama_program_select(&program, source, &access, false, &source_value);
    assert(status.code == AMA_OK);
    status = ama_program_select(&program, destination, &access, false, &destination_value);
    assert(status.code == AMA_OK);
    begin_transfer();
    bool no_op;
    status = ama_transfer_list_validate(&destination_value, &source_value, &operation, &no_op);
    assert(status.code == AMA_OK && !no_op && operation.protected_first == NULL);
    AmaHandleList prepared = {0};
    status = ama_transfer_list_prepare(&program, &destination_value, &source_value, NULL, &operation, &prepared);
    assert(status.code == AMA_OK && operation.first == NULL && cloned == 0);
    status = ama_transfer_list_commit(&program, &destination_value, &operation, &prepared);
    assert(status.code == AMA_OK && prepared.items == NULL && destination_value.list->count == 3);
    for (size_t index = 0; index < 3; index++) {
        assert(ama_handle_equal(destination_value.list->items[index], source_value.list->items[index]));
    }
    assert(busy->metadata.service.activity == AMA_INSTANCE_RUNNING && destroyed == 0);
    end_transfer();
    busy->metadata.service.activity = AMA_INSTANCE_READY;
    finish_transfer();
}

void run(void) {
    it_checks_aliases_shapes_and_continuation_self_copy_before_allocation();
    it_fixes_one_complete_private_forest_before_consuming_staging_at_commit();
    it_retains_staging_for_cleanup_on_parent_failure_and_destructive_failure();
    it_copies_reference_handles_including_busy_stale_and_empty_targets();
}

int main(void) {
    run();
    return 0;
}
