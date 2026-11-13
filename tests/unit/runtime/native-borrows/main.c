/**
 * @file
 * @brief Verify nominal native borrows, operation expiry and failure paths over real declared fields.
 */
#include "../objects/common.c"
#include "runtime/program/native/borrow.h"

static AmaProgram borrowed_program;
static AmaHandle borrowed_root;

/** Construct declared graph state before marking one explicit receiver Running with host authority. */
static void setup_borrows(void) {
    setup_objects();
    assert(ama_object_create(&world, 2, NULL, &borrowed_root).code == AMA_OK);
    borrowed_program = (AmaProgram){.world = &world, .current = borrowed_root};
    AmaPoolCell *cell = NULL;
    assert(ama_pool_lookup(&root_type.pool, borrowed_root, false, &cell).code == AMA_OK);
    cell->metadata.service.activity = AMA_INSTANCE_RUNNING;
    cell->metadata.service.return_link = (AmaReturnLink){.attached = true, .host = true};
}

/** Retire test execution authority, clean the full graph and assert zero counted allocations. */
static void finish_borrows(void) {
    assert(!world.operation_active);
    AmaPoolCell *cell = NULL;
    assert(ama_pool_lookup(&root_type.pool, borrowed_root, false, &cell).code == AMA_OK);
    cell->metadata.service.activity = AMA_INSTANCE_READY;
    cell->metadata.service.return_link = (AmaReturnLink){0};
    assert(ama_object_destroy(&world, borrowed_root).code == AMA_OK);
    destroy_objects();
}

/** Read already-bound owned child storage under an exclusive operation; never acquire the Busy receiver as a read. */
static void it_scopes_native_views_to_existing_bound_storage_and_rejects_reentry(void) {
    setup_borrows();
    RootValue *root = object_payload(borrowed_root);
    const void *original = &((LeafValue *)object_payload(root->left))->native;
    size_t before = allocation_count;
    AmaNativeBorrowScope scope = {0}, nested = {0};
    assert(ama_native_borrow_begin(&borrowed_program, borrowed_root, &scope).code == AMA_OK);
    assert(scope.active && world.operation_active && allocation_count == before);
    AmaPoolCell *cell = NULL;
    assert(ama_native_borrow_cell(&scope, borrowed_root, 2, AMA_NATIVE_BORROW_RECEIVER, &cell).code == AMA_OK);
    AmaPoolCell *receiver = cell;
    assert(ama_native_borrow_cell(&scope, borrowed_root, 2, AMA_NATIVE_BORROW_READ, &cell).code == AMA_BUSY && cell == receiver);
    assert(ama_native_borrow_cell(&scope, root->left, 1, AMA_NATIVE_BORROW_READ, &cell).code == AMA_OK);
    assert((unsigned char *)ama_pool_payload(&leaf_type.pool, cell) + leaf_type.native_offset == original);
    assert(ama_native_borrow_begin(&borrowed_program, borrowed_root, &nested).code == AMA_BUSY && !nested.active);
    assert(ama_object_destroy(&world, borrowed_root).code == AMA_BUSY);
    assert(ama_native_borrow_end(&scope).code == AMA_OK && !scope.active && scope.program == NULL);
    assert(!world.operation_active && allocation_count == before);
    assert(ama_native_borrow_cell(&scope, root->left, 1, AMA_NATIVE_BORROW_READ, &cell).code == AMA_INVALID_STATE);
    assert(ama_native_borrow_end(&scope).code == AMA_INVALID_STATE);
    assert(ama_native_borrow_begin(&borrowed_program, borrowed_root, &scope).code == AMA_OK);
    assert(ama_native_borrow_end(&scope).code == AMA_OK);
    finish_borrows();
}

/** Empty/stale/foreign-type/busy references and forged receiver authority expose no cell. */
static void it_checks_reference_identity_type_and_availability_before_view_publication(void) {
    setup_borrows();
    RootValue *root = object_payload(borrowed_root);
    root->selected = root->left;
    AmaHandle stale;
    assert(ama_object_create(&world, 1, NULL, &stale).code == AMA_OK);
    assert(ama_object_destroy(&world, stale).code == AMA_OK);
    AmaHandle foreign;
    assert(ama_object_create(&world, 2, NULL, &foreign).code == AMA_OK);
    AmaNativeBorrowScope scope = {0};
    assert(ama_native_borrow_begin(&borrowed_program, borrowed_root, &scope).code == AMA_OK);
    AmaPoolCell *output = NULL;
    assert(ama_native_borrow_cell(&scope, root->selected, 1, AMA_NATIVE_BORROW_READ, &output).code == AMA_OK);
    AmaPoolCell *unchanged = output;
    assert(ama_native_borrow_cell(&scope, (AmaHandle){0}, 1, AMA_NATIVE_BORROW_READ, &output).code == AMA_EMPTY_REFERENCE);
    assert(output == unchanged);
    assert(ama_native_borrow_cell(&scope, stale, 1, AMA_NATIVE_BORROW_READ, &output).code == AMA_STALE_REFERENCE);
    assert(output == unchanged);
    assert(ama_native_borrow_cell(&scope, foreign, 1, AMA_NATIVE_BORROW_READ, &output).code == AMA_TYPE_MISMATCH);
    assert(output == unchanged);
    assert(ama_native_borrow_cell(&scope, foreign, 2, AMA_NATIVE_BORROW_RECEIVER, &output).code == AMA_INVALID_STATE);
    assert(output == unchanged);
    AmaPoolCell *target = NULL;
    assert(ama_pool_lookup(&leaf_type.pool, root->selected, false, &target).code == AMA_OK);
    target->metadata.service.activity = AMA_INSTANCE_ACQUIRED;
    assert(ama_native_borrow_cell(&scope, root->selected, 1, AMA_NATIVE_BORROW_READ, &output).code == AMA_BUSY);
    assert(output == unchanged);
    target->metadata.service.activity = AMA_INSTANCE_READY;
    assert(ama_native_borrow_cell(&scope, root->selected, 0, AMA_NATIVE_BORROW_READ, &output).code == AMA_INVALID_STATE);
    assert(ama_native_borrow_cell(&scope, root->selected, 1, (AmaNativeBorrowRole)77, &output).code == AMA_INVALID_STATE);
    assert(output == unchanged);
    assert(ama_native_borrow_end(&scope).code == AMA_OK);
    assert(ama_object_destroy(&world, foreign).code == AMA_OK);
    finish_borrows();
}

/** No Ready, unattached, foreign-current or terminal context can manufacture receiver permission. */
static void it_rejects_invalid_receiver_authority_without_acquiring_a_gate(void) {
    setup_borrows();
    AmaPoolCell *cell = NULL;
    assert(ama_pool_lookup(&root_type.pool, borrowed_root, false, &cell).code == AMA_OK);
    AmaNativeBorrowScope scope = {0};
    cell->metadata.service.activity = AMA_INSTANCE_READY;
    assert(ama_native_borrow_begin(&borrowed_program, borrowed_root, &scope).code == AMA_INVALID_STATE);
    cell->metadata.service.activity = AMA_INSTANCE_RUNNING;
    cell->metadata.service.return_link = (AmaReturnLink){0};
    assert(ama_native_borrow_begin(&borrowed_program, borrowed_root, &scope).code == AMA_INVALID_STATE);
    cell->metadata.service.return_link = (AmaReturnLink){.attached = true, .host = true};
    borrowed_program.current = (AmaHandle){0};
    assert(ama_native_borrow_begin(&borrowed_program, borrowed_root, &scope).code == AMA_INVALID_STATE);
    borrowed_program.current = borrowed_root;
    world.failed = true;
    assert(ama_native_borrow_begin(&borrowed_program, borrowed_root, &scope).code == AMA_INVALID_STATE);
    world.failed = false;
    assert(!scope.active && scope.program == NULL && !world.operation_active);
    assert(ama_native_borrow_begin(NULL, borrowed_root, &scope).code == AMA_INVALID_STATE);
    assert(ama_native_borrow_begin(&borrowed_program, borrowed_root, NULL).code == AMA_INVALID_STATE);
    assert(ama_native_borrow_end(NULL).code == AMA_INVALID_STATE);
    finish_borrows();
}

/** Clone graph state independently; native clone hooks copy resources, while declared references fix up. */
static void it_borrows_cloned_native_state_without_aliasing_original_values(void) {
    setup_borrows();
    AmaPoolCell *receiver = NULL;
    assert(ama_pool_lookup(&root_type.pool, borrowed_root, false, &receiver).code == AMA_OK);
    receiver->metadata.service.activity = AMA_INSTANCE_READY;
    receiver->metadata.service.return_link = (AmaReturnLink){0};
    RootValue *root = object_payload(borrowed_root);
    root->selected = root->left;
    AmaHandle clone;
    assert(ama_object_clone(&world, borrowed_root, false, &clone).code == AMA_OK);
    RootValue *copy = object_payload(clone);
    assert(ama_handle_equal(copy->selected, copy->left) && !ama_handle_equal(copy->selected, root->selected));
    receiver->metadata.service.activity = AMA_INSTANCE_RUNNING;
    receiver->metadata.service.return_link = (AmaReturnLink){.attached = true, .host = true};
    AmaNativeBorrowScope scope = {0};
    assert(ama_native_borrow_begin(&borrowed_program, borrowed_root, &scope).code == AMA_OK);
    AmaPoolCell *original_cell = NULL, *copy_cell = NULL;
    assert(ama_native_borrow_cell(&scope, root->selected, 1, AMA_NATIVE_BORROW_READ, &original_cell).code == AMA_OK);
    assert(ama_native_borrow_cell(&scope, copy->selected, 1, AMA_NATIVE_BORROW_READ, &copy_cell).code == AMA_OK);
    LeafValue *original = ama_pool_payload(&leaf_type.pool, original_cell);
    LeafValue *independent = ama_pool_payload(&leaf_type.pool, copy_cell);
    assert(original->native.resource != independent->native.resource);
    size_t number = original->native.number;
    independent->native.number++;
    assert(original->native.number == number && independent->native.number == number + 1);
    assert(ama_native_borrow_end(&scope).code == AMA_OK);
    assert(ama_object_destroy(&world, clone).code == AMA_OK);
    finish_borrows();
}

/** Run explicit ownership, access, expiry and graph/native separation cases. */
void run(void) {
    it_scopes_native_views_to_existing_bound_storage_and_rejects_reentry();
    it_checks_reference_identity_type_and_availability_before_view_publication();
    it_rejects_invalid_receiver_authority_without_acquiring_a_gate();
    it_borrows_cloned_native_state_without_aliasing_original_values();
}

/** Succeed only after all real resource and access observations pass. */
int main(void) { run(); return EXIT_SUCCESS; }
