/**
 * @file
 * @brief Verify private construction phases, prefix cleanup and publication obligations.
 */
#include <stdint.h>
#include "../common.c"
#include "runtime/objects/construction/internal.h"

static AmaObjectOperation operation;

/** Own one nonreentrant operation's scratch/protection while phase tests run. */
static void begin_operation(void) {
    operation = (AmaObjectOperation){.world = &world};
    source_arena_init(&operation.arena, world.allocator);
    world.operation_active = true;
}

/** Release phase scratch/protection; published objects remain the test's responsibility. */
static void end_operation(void) {
    ama_object_unprotect(&operation);
    source_arena_destroy(&operation.arena);
    world.operation_active = false;
}

/** Failed pre-slot reservation exposes no work; post-slot failure registers cleanup. */
static void it_distinguishes_unreserved_failure_from_registered_partial_work(void) {
    setup_objects();
    begin_operation();
    AmaObjectWork *work = NULL;
    AmaStatus status = ama_construction_reserve(&operation, 99, NULL, (AmaHandle){0}, NULL, &work);
    assert(status.code == AMA_TYPE_MISMATCH && work == NULL && operation.first == NULL);
    failure_at = allocation_count + 1;
    status = ama_construction_reserve(&operation, 2, NULL, (AmaHandle){0}, NULL, &work);
    assert(status.code == AMA_OUT_OF_MEMORY && work == NULL && operation.first == NULL);
    failure_at = 0;
    AmaFieldRecipe field = {0};
    AmaRecipe invalid = {.fields = &field, .field_count = 1};
    status = ama_construction_reserve(&operation, 2, &invalid, (AmaHandle){0}, NULL, &work);
    assert(status.code == AMA_INVALID_STATE && work != NULL && operation.first == work);
    assert(!work->cell->metadata.identity.live && constructed == 0);
    AmaHandle handle = work->handle;
    status = ama_construction_unwind(&operation, work, status);
    assert(status.code == AMA_INVALID_STATE && destroyed == 0 && reports == 1);
    AmaPoolCell *cell = NULL;
    status = ama_pool_lookup(&root_type.pool, handle, true, &cell);
    assert(status.code != AMA_OK);
    end_operation();
    destroy_objects();
}

/** An uncredited finished child is cleaned before its uninitialized ancestor. */
static void it_keeps_child_ownership_and_prefix_cleanup_explicit(void) {
    setup_objects();
    begin_operation();
    AmaObjectWork *root = NULL;
    AmaStatus status = ama_construction_reserve(&operation, 2, NULL, (AmaHandle){0}, NULL, &root);
    assert(status.code == AMA_OK && constructed == 0);
    bool skipped;
    status = ama_construction_prepare_next(&operation, root, &skipped);
    assert(status.code == AMA_OK && !skipped && root->field_ready);
    assert(root->cell->metadata.lifetime.child_capacity == 1 && root->cell->metadata.lifetime.live_children == 0);
    AmaObjectWork *cursor = root;
    status = ama_construction_descend(&operation, &cursor);
    assert(status.code == AMA_OK && cursor != root && cursor->parent == root && constructed == 0);
    RootValue *value = ama_pool_payload(&root_type.pool, root->cell);
    assert(ama_handle_equal(value->left, cursor->handle));
    assert(ama_handle_equal(cursor->cell->owner, root->handle));
    for (size_t field = 0; field < leaf_type.field_count; field++) {
        status = ama_construction_prepare_next(&operation, cursor, &skipped);
        assert(status.code == AMA_OK && skipped);
    }
    status = ama_construction_payload(&operation, cursor);
    assert(status.code == AMA_OK && constructed == 1 && !cursor->cell->metadata.identity.live);
    status = ama_construction_unwind(&operation, cursor, ama_status_make(AMA_INVALID_LITERAL, NULL, 0));
    assert(status.code == AMA_INVALID_LITERAL && destroyed == 1 && reports == 1);
    end_operation();
    destroy_objects();
}

/** A complete private object becomes externally visible only in the publication phase. */
static void it_publishes_completed_state_without_moving_payloads(void) {
    setup_objects();
    begin_operation();
    AmaHandle handle = {0};
    AmaStatus status = ama_object_build(&operation, 1, NULL, &handle);
    assert(status.code == AMA_OK && constructed == 1 && leaf_type.pool.live_count == 0);
    AmaPoolCell *cell = NULL;
    status = ama_pool_lookup(&leaf_type.pool, handle, true, &cell);
    assert(status.code == AMA_OK && !cell->metadata.identity.live);
    void *address = ama_pool_payload(&leaf_type.pool, cell);
    status = ama_pool_lookup(&leaf_type.pool, handle, false, &cell);
    assert(status.code != AMA_OK);
    ama_construction_publish(&operation);
    status = ama_pool_lookup(&leaf_type.pool, handle, false, &cell);
    assert(status.code == AMA_OK && leaf_type.pool.live_count == 1);
    assert(ama_pool_payload(&leaf_type.pool, cell) == address);
    end_operation();
    status = ama_object_destroy(&world, handle);
    assert(status.code == AMA_OK && destroyed == 1);
    destroy_objects();
}

/** Field storage guards fail before any child/native constructor and clean buffers. */
static void it_rejects_unrepresentable_lists_and_wrong_reference_types_before_payload(void) {
    for (size_t mode = 0; mode < 2; mode++) {
        setup_objects();
        begin_operation();
        AmaHandle reference = {2, 0, 1};
        AmaFieldRecipe fields[6] = {0};
        fields[2].count = mode == 0 ? SIZE_MAX : 0;
        fields[5] = (AmaFieldRecipe){.count = 1, .references = &reference};
        AmaRecipe recipe = {.fields = fields, .field_count = 6};
        AmaObjectWork *work = NULL;
        AmaStatus status = ama_construction_reserve(&operation, 2, &recipe, (AmaHandle){0}, NULL, &work);
        assert(status.code == AMA_OK);
        work->next_field = mode == 0 ? 2 : 5;
        status = ama_construction_prepare_field(&operation, work);
        assert(status.code == (mode == 0 ? AMA_LIMIT_EXCEEDED : AMA_TYPE_MISMATCH));
        assert(constructed == 0);
        status = ama_construction_unwind(&operation, work, status);
        assert(status.code == (mode == 0 ? AMA_LIMIT_EXCEEDED : AMA_TYPE_MISMATCH));
        assert(destroyed == 0 && reports == 1);
        end_operation();
        destroy_objects();
    }
}

/** Publication preserves prepared continuation state and completed internal remapping. */
static void it_preserves_explicit_continuations_and_fixups_through_publication(void) {
    setup_objects();
    AmaHandle source;
    AmaStatus status = ama_object_create(&world, 2, NULL, &source);
    assert(status.code == AMA_OK);
    RootValue *original = object_payload(source);
    original->self = source;
    original->selected = original->left;
    AmaPoolCell *original_cell = NULL;
    status = ama_pool_lookup(&root_type.pool, source, false, &original_cell);
    assert(status.code == AMA_OK);
    original_cell->metadata.service.activity = AMA_INSTANCE_YIELDED;
    original_cell->metadata.service.resume = (AmaResumePosition){true, 77};
    begin_operation();
    AmaRecipe recipe = {.constructor = AMA_CONSTRUCT_CLONE, .source = source, .continuation = true};
    AmaHandle copy;
    status = ama_object_build(&operation, 2, &recipe, &copy);
    assert(status.code == AMA_OK);
    ama_construction_publish(&operation);
    end_operation();
    RootValue *value = object_payload(copy);
    assert(ama_handle_equal(value->self, copy) && ama_handle_equal(value->selected, value->left));
    AmaPoolCell *cell = NULL;
    status = ama_pool_lookup(&root_type.pool, copy, false, &cell);
    assert(status.code == AMA_OK && cell->metadata.service.resume.valid && cell->metadata.service.resume.selector == 77);
    assert(original_cell->metadata.service.activity == AMA_INSTANCE_YIELDED);
    status = ama_object_destroy(&world, copy);
    assert(status.code == AMA_OK);
    status = ama_object_destroy(&world, source);
    assert(status.code == AMA_OK);
    destroy_objects();
}

/** Run direct phase laws independently of generated code and language parsing. */
void run(void) {
    it_distinguishes_unreserved_failure_from_registered_partial_work();
    it_keeps_child_ownership_and_prefix_cleanup_explicit();
    it_publishes_completed_state_without_moving_payloads();
    it_rejects_unrepresentable_lists_and_wrong_reference_types_before_payload();
    it_preserves_explicit_continuations_and_fixups_through_publication();
}
/**
 * Run the explicit assertion suite and return success only after every selected case completes;
 * failed assertions preserve generated leaf evidence.
 */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
