/**
 * @file
 * @brief Test clone reservation, protected state, child rollback and copy boundaries.
 */
#include "../common.c"
#include "runtime/objects/clone/internal.h"

static AmaObjectOperation operation;

/** Hold the real world guard and source protections throughout private preparation. */
static void begin_operation(AmaHandle source) {
    operation = (AmaObjectOperation){.world = &world};
    source_arena_init(&operation.arena, world.allocator);
    world.operation_active = true;
    AmaStatus status = ama_object_preflight(&operation, source, true, false);
    assert(status.code == AMA_OK);
}

/** Restore source service state before releasing operation-owned scratch. */
static void end_operation(void) {
    ama_object_unprotect(&operation);
    source_arena_destroy(&operation.arena);
    world.operation_active = false;
}

/** Every measured reservation allocation prefix either owns registered work or no slot. */
static void it_exposes_registered_work_on_late_field_failure_and_releases_each_prefix(void) {
    size_t count = 0;
    bool late_failure = false;
    for (size_t failure = 0; failure <= count; failure++) {
        setup_objects();
        AmaFieldRecipe fields[6] = {0};
        fields[5].count = 2;
        AmaRecipe recipe = {.fields = fields, .field_count = 6};
        AmaHandle source;
        AmaStatus status = ama_object_create(&world, 2, &recipe, &source);
        assert(status.code == AMA_OK);
        begin_operation(source);
        size_t start = allocation_count;
        failure_at = failure == 0 ? 0 : start + failure;
        AmaObjectWork *work = NULL;
        status = ama_clone_reserve(&operation, source, NULL, NULL, (AmaHandle){0}, &work);
        if (failure == 0) {
            assert(status.code == AMA_OK && work != NULL);
            count = allocation_count - start;
        } else {
            assert(status.code == AMA_OUT_OF_MEMORY);
            late_failure |= work != NULL;
        }
        assert((operation.first != NULL) == (work != NULL));
        if (work != NULL) {
            assert(!work->cell->metadata.identity.live && work->clone_root == work);
            AmaStatus cleaned = ama_object_cleanup(&world, work->handle, status);
            assert(cleaned.code == status.code);
        }
        failure_at = 0;
        assert(cloned == 0 && destroyed == 0);
        end_operation();
        assert(((RootValue *)object_payload(source))->native.resource != NULL);
        status = ama_object_destroy(&world, source);
        assert(status.code == AMA_OK);
        destroy_objects();
    }
    assert(count != 0 && late_failure);
}

/** A child reservation belongs to its parent even before the live prefix advances. */
static void it_installs_the_child_identity_before_crediting_success(void) {
    setup_objects();
    AmaHandle source;
    AmaStatus status = ama_object_create(&world, 2, NULL, &source);
    assert(status.code == AMA_OK);
    begin_operation(source);
    AmaObjectWork *root = NULL;
    status = ama_clone_reserve(&operation, source, NULL, NULL, (AmaHandle){0}, &root);
    assert(status.code == AMA_OK);
    AmaObjectWork *cursor = root;
    status = ama_clone_descend(&operation, &cursor);
    assert(status.code == AMA_OK && cursor->parent == root && cursor->clone_root == root);
    assert(ama_handle_equal(cursor->cell->owner, root->handle));
    RootValue *value = ama_pool_payload(&root_type.pool, root->cell);
    assert(ama_handle_equal(value->left, cursor->handle));
    assert(root->cell->metadata.lifetime.live_children == 0);
    status = ama_object_clone_payload(&operation, cursor, false);
    assert(status.code == AMA_OK && cloned == 1 && cursor->cell->metadata.lifetime.native_live);
    status = ama_object_cleanup(&world, cursor->handle, ama_status_success());
    assert(status.code == AMA_OK && destroyed == 1);
    status = ama_object_cleanup(&world, root->handle, status);
    assert(status.code == AMA_OK && destroyed == 1);
    end_operation();
    status = ama_object_destroy(&world, source);
    assert(status.code == AMA_OK);
    destroy_objects();
}

/** Separate copies of one source never merge; replacement redirects only its root. */
static void it_scopes_reference_fixups_and_final_root_redirection_to_one_copy_boundary(void) {
    setup_objects();
    AmaHandle source, final;
    AmaStatus status = ama_object_create(&world, 1, NULL, &source);
    assert(status.code == AMA_OK);
    status = ama_object_create(&world, 1, NULL, &final);
    assert(status.code == AMA_OK);
    ((LeafValue *)object_payload(source))->peer = source;
    begin_operation(source);
    AmaObjectWork *first = NULL, *second = NULL;
    status = ama_clone_reserve(&operation, source, NULL, NULL, (AmaHandle){0}, &first);
    assert(status.code == AMA_OK);
    status = ama_clone_reserve(&operation, source, NULL, NULL, (AmaHandle){0}, &second);
    assert(status.code == AMA_OK && first->clone_root != second->clone_root);
    status = ama_object_clone_payload(&operation, first, false);
    assert(status.code == AMA_OK);
    status = ama_object_clone_payload(&operation, second, false);
    assert(status.code == AMA_OK);
    LeafValue *left = ama_pool_payload(&leaf_type.pool, first->cell);
    LeafValue *right = ama_pool_payload(&leaf_type.pool, second->cell);
    ama_object_fix_references(&operation, first, final);
    assert(ama_handle_equal(left->peer, final) && ama_handle_equal(right->peer, source));
    ama_object_fix_references(&operation, second, (AmaHandle){0});
    assert(ama_handle_equal(right->peer, second->handle) && !ama_handle_equal(first->handle, second->handle));
    assert(ama_handle_equal(((LeafValue *)object_payload(source))->peer, source));
    status = ama_object_cleanup(&world, second->handle, ama_status_success());
    assert(status.code == AMA_OK);
    status = ama_object_cleanup(&world, first->handle, status);
    assert(status.code == AMA_OK);
    end_operation();
    status = ama_object_destroy(&world, final);
    assert(status.code == AMA_OK);
    status = ama_object_destroy(&world, source);
    assert(status.code == AMA_OK);
    destroy_objects();
}

/** Each native failure destroys only the successful clone prefix, preserving source. */
static void it_rolls_back_each_native_failure_without_destroying_the_source_or_failed_payload(void) {
    for (size_t failure = 1; failure <= 3; failure++) {
        setup_objects();
        AmaHandle source;
        AmaStatus status = ama_object_create(&world, 2, NULL, &source);
        assert(status.code == AMA_OK);
        void *resource = ((RootValue *)object_payload(source))->native.resource;
        failed_constructor = constructed + failure;
        begin_operation(source);
        AmaHandle result = {0};
        status = ama_object_clone_prepare(&operation, source, (AmaHandle){0}, false, NULL, &result);
        assert(status.code == AMA_INVALID_LITERAL && result.generation == 0);
        assert(cloned == failure && destroyed == failure - 1 && reports == 1);
        end_operation();
        assert(((RootValue *)object_payload(source))->native.resource == resource);
        assert(root_type.pool.live_count == 1 && leaf_type.pool.live_count == 2);
        status = ama_object_destroy(&world, source);
        assert(status.code == AMA_OK && destroyed == failure + 2);
        destroy_objects();
    }
}

/**
 * Execute every named assertion case selected by this leaf; the recursive harness owns process
 * status and success-only cleanup.
 */
void run(void) {
    it_exposes_registered_work_on_late_field_failure_and_releases_each_prefix();
    it_installs_the_child_identity_before_crediting_success();
    it_scopes_reference_fixups_and_final_root_redirection_to_one_copy_boundary();
    it_rolls_back_each_native_failure_without_destroying_the_source_or_failed_payload();
}

/**
 * Run the explicit assertion suite and return success only after every selected case completes;
 * failed assertions preserve generated leaf evidence.
 */
int main(void) {
    run();
    return 0;
}
