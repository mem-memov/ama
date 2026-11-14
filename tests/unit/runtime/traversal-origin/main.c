/** @file
 * @brief Enforce removed-origin guards, deliberate detachment and lexical resume order.
 */
#include "common.c"

/** Removed occurrences refuse every target mode before inputs, resets or native
 * conversion; a separately copied ordinary reference still reaches its live target.
 */
static void it_guards_removed_bindings_before_target_access_and_decoder_effects(void) {
    setup_origins(false, false);
    select_origin(0, 1);
    AmaProgramAccess binding = origin_access();
    bool initial_truth = false;
    checked(ama_program_condition(&program, owners[0], &binding, &initial_truth));
    assert(initial_truth && conversions == 1);
    conversions = 0;
    AmaProgramValue selected;
    checked(ama_program_select(&program, owners[0], &binding, false, &selected));
    const AmaProgramAccess independent = {.field = 2};
    checked(ama_program_transfer(&program, owners[0], &independent, &selected));
    remove_origin(0);
    for (AmaAccessMode mode = AMA_ACCESS_ENTRY; mode <= AMA_ACCESS_READ; mode++) {
        const AmaProgramInvocation invocation = {.target = &binding, .mode = mode};
        bool refused = false;
        AmaStatus status = ama_program_acquire(&program, owners[0], &invocation, &refused);
        assert(status.code == AMA_STALE_REFERENCE && refused && !owner_value(0)->frame.active);
    }
    bool truth = false;
    AmaStatus status = ama_program_condition(&program, owners[0], &binding, &truth);
    assert(status.code == AMA_STALE_REFERENCE && !truth && conversions == 0);
    const AmaProgramAccess indexed = {.field = 3, .index = &binding};
    const AmaProgramInvocation indexed_call = {.target = &indexed, .mode = AMA_ACCESS_READ};
    bool refused = false;
    status = ama_program_acquire(&program, owners[0], &indexed_call, &refused);
    assert(status.code == AMA_STALE_REFERENCE && refused && conversions == 0);
    const AmaProgramInvocation ordinary = {.target = &independent, .mode = AMA_ACCESS_READ};
    checked(ama_program_acquire(&program, owners[0], &ordinary, &refused));
    assert(!refused);
    checked(ama_program_release(&program, owners[0]));
    assert(ama_handle_equal(owner_value(0)->independent, owners[1]));
    finish_origins();
}

/** Same-handle writes retain origin; a different explicit assignment detaches
 * both a local and a reused reference without changing the scan boundary.
 */
static void it_detaches_only_different_explicit_assignments_without_moving_boundaries(void) {
    for (size_t local = 0; local < 2; local++) {
        setup_origins(local != 0, false);
        select_origin(0, 1);
        remove_origin(0);
        AmaProgramAccess access = origin_access();
        AmaProgramValue source = {.type_id = 1, .form = AMA_FIELD_REFERENCE, .handle = owners[1]};
        checked(ama_program_transfer(&program, owners[0], &access, &source));
        assert(owner_value(0)->binding.associated);
        const AmaProgramInvocation invocation = {.target = &access, .mode = AMA_ACCESS_READ};
        bool refused;
        AmaStatus status = ama_program_acquire(&program, owners[0], &invocation, &refused);
        assert(status.code == AMA_STALE_REFERENCE && refused);
        source.handle = owners[2];
        checked(ama_program_transfer(&program, owners[0], &access, &source));
        assert(!owner_value(0)->binding.associated && owner_value(0)->binding.roles == 1
            && owner_value(0)->cursors[0].boundary == 0);
        checked(ama_program_acquire(&program, owners[0], &invocation, &refused));
        assert(!refused);
        checked(ama_program_release(&program, owners[0]));
        checked(ama_program_foreach_reset(&program, owners[0]));
        if (local == 0) { assert(ama_handle_equal(owner_value(0)->reused, owners[2])); }
        else {
            AmaProgramValue value;
            status = ama_program_select(&program, owners[0], &access, false, &value);
            assert(status.code == AMA_INVALID_STATE);
        }
        finish_origins();
    }
}

/** A direct structural helper can borrow its exact busy-list grant; a normal
 * call through the same binding cannot inherit that grant or acquire the target.
 */
static void it_checks_busy_origins_with_only_exact_method_direct_authority(void) {
    setup_origins(false, false);
    select_origin(0, 1);
    AmaProgramAccess access = origin_access();
    AmaProgramValue value;
    checked(ama_program_select(&program, owners[0], &access, false, &value));
    bool refused;
    checked(ama_program_list_open(&program, owners[0], owners[0], 0, &owner_value(0)->service, &refused));
    AmaStatus status = ama_program_foreach_guard(&program, &value, NULL, &refused);
    assert(status.code == AMA_BUSY && refused);
    checked(ama_program_foreach_guard(&program, &value, &owner_value(0)->service, &refused));
    assert(!refused);
    const AmaProgramInvocation invocation = {.target = &access, .mode = AMA_ACCESS_READ};
    status = ama_program_acquire(&program, owners[0], &invocation, &refused);
    assert(status.code == AMA_BUSY && refused);
    checked(ama_program_list_close(&owner_value(0)->service));
    checked(ama_program_acquire(&program, owners[0], &invocation, &refused));
    checked(ama_program_release(&program, owners[0]));
    finish_origins();
}

/** Owning append checks the staged origin during its documented preflight;
 * reference append merely copies identity and creates an independent occurrence.
 */
static void it_checks_owning_clone_origins_without_restricting_plain_handle_copies(void) {
    setup_origins(false, false);
    select_origin(0, 1);
    remove_origin(0);
    AmaProgramAccess access = origin_access();
    const AmaProgramInput input = {.field = 0, .source = {.kind = AMA_SOURCE_LOCAL, .access = &access}};
    const AmaProgramListInputs plan = {.inputs = &input, .count = 1, .integer_type = 1};
    OriginOwner *owner = owner_value(0);
    bool refused;
    checked(ama_program_list_open(&program, owners[0], owners[0], 4, &owner->service, &refused));
    size_t allocations = allocation_count;
    AmaStatus status = ama_program_list_append(&owner->service, &owner->method, &plan, &refused);
    assert(status.code == AMA_STALE_REFERENCE && refused && owner->owned.count == 0
        && owner_type.pool.live_count == 3 && allocation_count == allocations && !world.failed);
    checked(ama_program_list_method_end(&owner->method));
    checked(ama_program_list_close(&owner->service));
    checked(ama_program_list_open(&program, owners[0], owners[0], 3, &owner->service, &refused));
    checked(ama_program_list_append(&owner->service, &owner->method, &plan, &refused));
    assert(!refused && owner->other.count == 2 && ama_handle_equal(owner->method.value.handle, owners[1])
        && owner->method.value.foreach_binding == NULL);
    checked(ama_program_list_method_end(&owner->method));
    checked(ama_program_list_close(&owner->service));
    finish_origins();
}

/** An absent outer current occurrence discards all inner roles before directing
 * the caller to the outer corrected boundary; no saved body statement is selected.
 */
static void it_abandons_nested_scopes_before_advancing_a_missing_outer_iteration(void) {
    setup_origins(false, false);
    select_origin(0, 1);
    select_origin(1, 2);
    remove_origin(0);
    uint64_t repeat = UINT64_MAX;
    checked(ama_program_foreach_resume(&program, owners[0], &repeat));
    assert(repeat == 101 && owner_value(0)->cursors[0].active && !owner_value(0)->cursors[1].active
        && owner_value(0)->cursors[0].boundary == 0 && owner_value(0)->binding.roles == 1);
    bool selected;
    checked(ama_program_foreach_next(&program, owners[0], &scopes[0].plan, &selected));
    assert(selected && ama_handle_equal(owner_value(0)->binding.installed, owners[2]));
    finish_origins();
}

/** The outer membership validates independently of the shared cell overwritten
 * by its inner role. Closing the missing inner scan never restores the outer origin.
 */
static void it_preserves_last_nested_origin_without_restoring_an_outer_binding(void) {
    setup_origins(false, false);
    select_origin(0, 1);
    select_origin(1, 2);
    remove_origin(1);
    uint64_t repeat;
    checked(ama_program_foreach_resume(&program, owners[0], &repeat));
    assert(repeat == 201 && owner_value(0)->binding.roles == 2);
    bool selected = true;
    checked(ama_program_foreach_next(&program, owners[0], &scopes[1].plan, &selected));
    assert(!selected && owner_value(0)->binding.roles == 1
        && ama_handle_equal(owner_value(0)->reused, owners[2]) && owner_value(0)->binding.associated);
    AmaProgramAccess access = origin_access();
    const AmaProgramInvocation invocation = {.target = &access, .mode = AMA_ACCESS_READ};
    bool refused;
    AmaStatus status = ama_program_acquire(&program, owners[0], &invocation, &refused);
    assert(status.code == AMA_STALE_REFERENCE && refused);
    finish_origins();
}

/** Collection replacement is a technical failure at the first invalid retained
 * header, never an instruction to continue a saved body or silently restart.
 */
static void it_reports_replaced_collections_at_the_correct_outermost_header(void) {
    for (size_t separate = 0; separate < 2; separate++) {
        setup_origins(false, separate != 0);
        select_origin(0, 1);
        select_origin(1, 1);
        checked(ama_membership_reset(separate != 0 ? &owner_value(0)->other : &owner_value(0)->items));
        uint64_t repeat = UINT64_MAX;
        AmaStatus status = ama_program_foreach_resume(&program, owners[0], &repeat);
        AmaPoolCell *cell = NULL;
        checked(ama_program_cell(&program, owners[0], &cell));
        assert(status.code == AMA_STALE_REFERENCE && repeat == 0
            && cell->metadata.service.current_operation == (separate != 0 ? 200u : 100u));
        finish_origins();
    }
}

/** A deleted referent invalidates the current iteration even while its original
 * occurrence remains; the following live candidate is selected from the saved gap.
 */
static void it_advances_after_a_current_referent_is_deleted_and_its_slot_reused(void) {
    setup_origins(true, false);
    select_origin(0, 1);
    checked(ama_object_destroy(&world, owners[1]));
    checked(ama_object_create(&world, 1, NULL, &owners[1]));
    uint64_t repeat;
    checked(ama_program_foreach_resume(&program, owners[0], &repeat));
    assert(repeat == 101);
    bool selected;
    checked(ama_program_foreach_next(&program, owners[0], &scopes[0].plan, &selected));
    assert(selected && ama_handle_equal(owner_value(0)->binding.installed, owners[2]));
    finish_origins();
}

/** Internal list-acquisition failure on resume is technical at the header, and
 * does not release another live service grant or select a repeat/body branch.
 */
static void it_preserves_existing_grants_when_resume_cannot_acquire_a_collection(void) {
    setup_origins(false, false);
    select_origin(0, 1);
    bool refused;
    checked(ama_program_list_open(&program, owners[0], owners[0], 0, &owner_value(0)->service, &refused));
    uint64_t repeat = UINT64_MAX;
    AmaStatus status = ama_program_foreach_resume(&program, owners[0], &repeat);
    assert(status.code == AMA_BUSY && repeat == 0 && owner_value(0)->items.acquisition == &owner_value(0)->service);
    checked(ama_program_list_close(&owner_value(0)->service));
    finish_origins();
}

/** Invalid scope parents and incompatible reference cells fail static schema
 * checks without allocating, activating a role or accessing a source list.
 */
static void it_rejects_invalid_scope_preorder_and_incompatible_binding_storage(void) {
    setup_origins(false, false);
    AmaProgramForeachScope invalid[2] = {scopes[0], scopes[1]};
    invalid[1].parent = 1;
    size_t allocations = allocation_count;
    AmaStatus status = ama_program_foreach_schema(sizeof(OriginOwner), owner_type.fields, 5, invalid, 2);
    assert(status.code == AMA_INVALID_STATE && allocation_count == allocations);
    invalid[1] = scopes[1];
    invalid[0].plan.value_offset = offsetof(OriginOwner, owned);
    status = ama_program_foreach_schema(sizeof(OriginOwner), owner_type.fields, 5, invalid, 2);
    assert(status.code == AMA_TYPE_MISMATCH && allocation_count == allocations);
    finish_origins();
}

/** Valid retained scans preserve saved dispatch and shared origin, even when the
 * current referent is busy; fresh scope reset clears only lexical state/metadata.
 */
static void it_preserves_valid_resume_dispatch_and_discards_scopes_without_target_cleanup(void) {
    for (size_t local = 0; local < 2; local++) {
        setup_origins(local != 0, false);
        select_origin(0, 1);
        select_origin(1, 2);
        AmaPoolCell *owner = NULL, *target = NULL;
        checked(ama_program_cell(&program, owners[0], &owner));
        checked(ama_program_cell(&program, owners[2], &target));
        owner->metadata.service.resume = (AmaResumePosition){.valid = true, .selector = 444};
        target->metadata.service.activity = AMA_INSTANCE_WAITING;
        uint64_t repeat = UINT64_MAX;
        checked(ama_program_foreach_resume(&program, owners[0], &repeat));
        assert(repeat == 0 && owner->metadata.service.resume.selector == 444
            && owner_value(0)->binding.roles == 2 && owner_value(0)->binding.associated
            && owner_value(0)->items.acquisition == NULL);
        AmaProgramAccess access = origin_access();
        const AmaProgramInvocation invocation = {.target = &access, .mode = AMA_ACCESS_READ};
        bool refused;
        AmaStatus status = ama_program_acquire(&program, owners[0], &invocation, &refused);
        assert(status.code == AMA_BUSY && refused);
        checked(ama_program_foreach_reset(&program, owners[0]));
        assert(owner_value(0)->binding.roles == 0 && !owner_value(0)->binding.associated
            && owner_type.pool.live_count == 3 && !owner_value(0)->cursors[0].active && !owner_value(0)->cursors[1].active);
        if (local == 0) { assert(ama_handle_equal(owner_value(0)->reused, owners[2])); }
        owner->metadata.service.resume = (AmaResumePosition){0};
        finish_origins();
    }
}
/** Run target/ref-copy distinctions and retained lexical resume decisions explicitly. */
void run(void) {
    it_guards_removed_bindings_before_target_access_and_decoder_effects();
    it_detaches_only_different_explicit_assignments_without_moving_boundaries();
    it_checks_busy_origins_with_only_exact_method_direct_authority();
    it_checks_owning_clone_origins_without_restricting_plain_handle_copies();
    it_abandons_nested_scopes_before_advancing_a_missing_outer_iteration();
    it_preserves_last_nested_origin_without_restoring_an_outer_binding();
    it_reports_replaced_collections_at_the_correct_outermost_header();
    it_advances_after_a_current_referent_is_deleted_and_its_slot_reused();
    it_preserves_existing_grants_when_resume_cannot_acquire_a_collection();
    it_rejects_invalid_scope_preorder_and_incompatible_binding_storage();
    it_preserves_valid_resume_dispatch_and_discards_scopes_without_target_cleanup();
}
/** Return only after every origin role, stable root and counted allocation is gone. */
int main(void) { run(); return EXIT_SUCCESS; }
