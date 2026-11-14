/** @file
 * @brief Test exact list grant borrowing, sibling lifetime and owner protection.
 */
#include "common.c"

/** Sibling methods share one busy interval; copied grants and executed callers
 * cannot borrow authority merely because they know the same owner identity.
 */
static void it_keeps_siblings_busy_and_borrows_only_the_exact_parent_grant(void) {
    setup_service();
    ServiceOwner *owner = owner_value(0);
    AmaProgramListService *service = &owner->services[0];
    bool refused = true;
    AmaStatus status = ama_program_list_open(&program, owners[0], owners[0], 0, service, &refused);
    assert(status.code == AMA_OK && !refused && owner->items.acquisition == service);
    status = ama_program_list_open(&program, owners[0], owners[0], 0, &owner->services[1], &refused);
    assert(status.code == AMA_BUSY && refused && !owner->services[1].active);
    const AmaProgramAccess access = {.field = 0};
    AmaProgramValue value = {.type_id = 99};
    status = ama_program_select_target(&program, owners[0], &access, &value, &refused);
    assert(status.code == AMA_BUSY && refused && value.type_id == 99);
    status = ama_program_access_authorized(&program, owners[0], owners[0], NULL,
        &access, false, service, &value, &refused);
    assert(status.code == AMA_OK && !refused && value.list == &owner->items);
    status = ama_program_access_authorized(&program, owners[2], owners[0], NULL,
        &access, false, service, &value, &refused);
    assert(status.code == AMA_INVALID_STATE && !refused && value.list == &owner->items);
    AmaProgramListService copied = *service;
    status = ama_program_access_authorized(&program, owners[0], owners[0], NULL,
        &access, false, &copied, &value, &refused);
    assert(status.code == AMA_BUSY && refused);
    program.current = owners[2];
    status = ama_program_access_authorized(&program, owners[2], owners[0], NULL,
        &access, false, service, &value, &refused);
    assert(status.code == AMA_BUSY && refused);
    program.current = owners[0];
    status = ama_program_list_open(&program, owners[0], owners[0], 1, &owner->services[1], &refused);
    assert(status.code == AMA_OK);
    const AmaProgramAccess other = {.field = 1};
    status = ama_program_access_authorized(&program, owners[0], owners[0], NULL,
        &other, false, service, &value, &refused);
    assert(status.code == AMA_BUSY && refused);
    status = ama_program_access_authorized(&program, owners[0], owners[0], NULL,
        &other, false, &owner->services[1], &value, &refused);
    assert(status.code == AMA_OK && !refused);
    status = ama_program_list_close(&owner->services[1]);
    assert(status.code == AMA_OK && owner->items.acquisition == service);
    for (size_t index = 0; index < 2; index++) {
        status = ama_program_list_method_begin(service, &owner->methods[index]);
        assert(status.code == AMA_OK && owner->items.acquisition == service);
        AmaProgramListMethod copied_method = owner->methods[index];
        status = ama_program_list_reference_result(&copied_method, (AmaHandle){0});
        assert(status.code == AMA_INVALID_STATE);
        status = ama_program_list_method_begin(service, &owner->methods[1 - index]);
        assert(status.code == AMA_INVALID_STATE);
        status = ama_program_list_close(service);
        assert(status.code == AMA_INVALID_STATE);
        status = ama_program_list_method_end(&owner->methods[index]);
        assert(status.code == AMA_OK && owner->items.acquisition == service);
    }
    status = ama_program_list_close(service);
    assert(status.code == AMA_OK && owner->items.acquisition == NULL);
    status = ama_program_select(&program, owners[0], &access, false, &value);
    assert(status.code == AMA_OK);
    finish_service();
}

/** A selected parent's acquisition is borrowed, not restored/released by list
 * close. Another alias still refuses ordinary module access while it is held.
 */
static void it_retains_selected_owner_protection_and_rejects_alias_reentry(void) {
    setup_service();
    const AmaProgramAccess parent = {.field = 2};
    const AmaProgramInvocation read = {.target = &parent, .mode = AMA_ACCESS_READ};
    bool refused;
    AmaStatus status = ama_program_acquire(&program, owners[0], &read, &refused);
    assert(status.code == AMA_OK && !refused);
    AmaProgramListService *service = &owner_value(0)->services[0];
    status = ama_program_list_open(&program, owners[0], owners[1], 0, service, &refused);
    assert(status.code == AMA_OK && !refused);
    status = ama_object_destroy(&world, owners[1]);
    assert(status.code == AMA_BUSY);
    owner_activity(2, AMA_INSTANCE_RUNNING);
    program.current = owners[2];
    status = ama_program_acquire(&program, owners[2], &read, &refused);
    assert(status.code == AMA_BUSY && refused);
    program.current = owners[0];
    status = ama_program_list_close(service);
    assert(status.code == AMA_OK);
    AmaPoolCell *cell = NULL;
    status = ama_program_cell(&program, owners[1], &cell);
    assert(status.code == AMA_OK && cell->metadata.service.activity == AMA_INSTANCE_ACQUIRED);
    status = ama_program_release(&program, owners[0]);
    assert(status.code == AMA_OK && cell->metadata.service.activity == AMA_INSTANCE_READY);
    finish_service();
}
