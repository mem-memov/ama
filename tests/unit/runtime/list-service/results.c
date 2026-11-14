/** @file
 * @brief Test ephemeral result credit, stale escapes and callback-safe cleanup.
 */
#include "common.c"

/** Select one literal constructor and expire escaped references after the full
 * result lifetime. Callback reentry cannot shorten the parent busy interval.
 */
static void it_finalizes_owned_results_once_and_expires_escaped_handles(void) {
    setup_service();
    ServiceOwner *owner = owner_value(0);
    bool refused;
    AmaStatus status = ama_program_list_open(&program, owners[0], owners[0], 0, &owner->services[0], &refused);
    assert(status.code == AMA_OK);
    status = ama_program_list_method_begin(&owner->services[0], &owner->methods[0]);
    assert(status.code == AMA_OK);
    const unsigned char literal[] = {'x'};
    const AmaRecipe recipe = {.constructor = AMA_CONSTRUCT_LITERAL, .literal = {literal, 1}};
    status = ama_program_list_owned_result(&owner->methods[0], 1, &recipe);
    assert(status.code == AMA_OK && defaults == 0 && literals == 1 && drops == 0);
    AmaHandle escaped = owner->methods[0].value.handle;
    status = ama_program_list_owned_result(&owner->methods[0], 1, NULL);
    assert(status.code == AMA_INVALID_STATE && defaults == 0);
    check_callback = true;
    status = ama_program_list_method_end(&owner->methods[0]);
    assert(status.code == AMA_OK && drops == 1 && owner->items.acquisition != NULL);
    status = ama_program_list_method_end(&owner->methods[0]);
    assert(status.code == AMA_INVALID_STATE && drops == 1);
    AmaPoolCell *cell = NULL;
    status = ama_program_cell(&program, escaped, &cell);
    assert(status.code == AMA_STALE_REFERENCE);
    status = ama_program_list_close(&owner->services[0]);
    assert(status.code == AMA_OK);
    finish_service();
}

/** Reference results neither acquire nor destroy their possibly busy targets. */
static void it_keeps_reference_results_nonowning_and_preserves_target_activity(void) {
    setup_service();
    AmaHandle target;
    AmaStatus status = ama_object_create(&world, 1, NULL, &target);
    assert(status.code == AMA_OK && defaults == 1);
    AmaPoolCell *cell = NULL;
    status = ama_program_cell(&program, target, &cell);
    assert(status.code == AMA_OK);
    cell->metadata.service.activity = AMA_INSTANCE_RUNNING;
    ServiceOwner *owner = owner_value(0);
    bool refused;
    status = ama_program_list_open(&program, owners[0], owners[0], 0, &owner->services[0], &refused);
    assert(status.code == AMA_OK);
    status = ama_program_list_method_begin(&owner->services[0], &owner->methods[0]);
    assert(status.code == AMA_OK);
    status = ama_program_list_reference_result(&owner->methods[0], target);
    assert(status.code == AMA_OK && !owner->methods[0].owns_result);
    assert(ama_handle_equal(owner->methods[0].value.handle, target));
    status = ama_program_list_method_end(&owner->methods[0]);
    assert(status.code == AMA_OK && drops == 0 && cell->metadata.service.activity == AMA_INSTANCE_RUNNING);
    status = ama_program_list_close(&owner->services[0]);
    assert(status.code == AMA_OK);
    cell->metadata.service.activity = AMA_INSTANCE_READY;
    status = ama_object_destroy(&world, target);
    assert(status.code == AMA_OK && drops == 1);
    finish_service();
}

/** Failed construction creates no destroy credit; destructive failure consumes
 * its credit and still allows releasing the exact parent grant during cleanup.
 */
static void it_cleans_failed_results_without_repeating_destruction_or_leaking_grants(void) {
    for (size_t destructive = 0; destructive < 2; destructive++) {
        setup_service();
        ServiceOwner *owner = owner_value(0);
        bool refused;
        AmaStatus status = ama_program_list_open(&program, owners[0], owners[0], 0, &owner->services[0], &refused);
        assert(status.code == AMA_OK);
        status = ama_program_list_method_begin(&owner->services[0], &owner->methods[0]);
        assert(status.code == AMA_OK);
        constructor_failure = !destructive;
        status = ama_program_list_owned_result(&owner->methods[0], 1, NULL);
        assert(status.code == (destructive ? AMA_OK : AMA_NATIVE_FAILURE));
        assert(owner->methods[0].has_result == (destructive != 0));
        destructor_failure = destructive != 0;
        status = ama_program_list_method_end(&owner->methods[0]);
        assert(status.code == (destructive ? AMA_IO_FAILURE : AMA_OK));
        assert(drops == destructive && !owner->methods[0].active);
        status = ama_program_list_close(&owner->services[0]);
        assert(status.code == AMA_OK && owner->items.acquisition == NULL);
        finish_service();
    }
}

/** Typed frame access validates alignment and extent before publishing a pointer. */
static void it_borrows_only_aligned_bounded_lexical_frame_slots(void) {
    setup_service();
    ServiceOwner *owner = owner_value(0);
    AmaProgramListService *service = NULL;
    AmaProgramListMethod *method = NULL;
    AmaStatus status = ama_program_list_service_frame(&program, owners[0], offsetof(ServiceOwner, services), &service);
    assert(status.code == AMA_OK && service == &owner->services[0]);
    status = ama_program_list_method_frame(&program, owners[0], offsetof(ServiceOwner, methods), &method);
    assert(status.code == AMA_OK && method == &owner->methods[0]);
    status = ama_program_list_service_frame(&program, owners[0], SIZE_MAX, &service);
    assert(status.code == AMA_INVALID_STATE && service == &owner->services[0]);
    status = ama_program_list_method_frame(&program, owners[0], 1, &method);
    assert(status.code == AMA_INVALID_STATE && method == &owner->methods[0]);
    finish_service();
}
