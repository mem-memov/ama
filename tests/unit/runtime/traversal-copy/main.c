/** @file
 * @brief Verify exact membership maps, permanently dead origins and copy independence.
 */
#include "common.c"
#include "native.c"
#include "runtime/program/transfers/list/internal.h"

/** A live occurrence maps to its new namespace; future edits correct only the
 * corresponding source/copy cursors, and no operation or list grant is copied.
 */
static void it_copies_live_boundaries_into_independent_namespaces_and_control_cells(void) {
    setup_copy(false, false);
    select_origin(0, 1);
    select_origin(1, 2);
    yield_copy_source();
    AmaHandle copied;
    checked(ama_object_clone(&world, owners[0], true, &copied));
    OriginOwner *original = owner_value(0), *clone = copy_value(copied);
    assert(clone != original && clone->cursors[0].active && clone->cursors[1].active
        && clone->cursors[0].boundary == 1 && clone->cursors[1].boundary == 2 && clone->binding.roles == 2);
    assert(ama_handle_equal(clone->cursors[0].owner, copied)
        && ama_handle_equal(clone->cursors[0].current.owner, copied)
        && ama_handle_equal(clone->binding.origin.owner, copied)
        && clone->cursors[0].incarnation == clone->items.incarnation
        && !clone->frame.active && !clone->service.active && !clone->method.active && clone->items.acquisition == NULL);
    assert(inspect_copy_resume(copied) == 0);
    AmaListStorage changed = {&world, copied, &clone->items, 1, false};
    checked(ama_list_storage_insert(&changed, 0, owners[1]));
    assert(clone->cursors[0].boundary == 2 && clone->cursors[1].boundary == 3
        && original->cursors[0].boundary == 1 && original->cursors[1].boundary == 2);
    AmaListStorage source = {&world, owners[0], &original->items, 1, false};
    checked(ama_list_storage_remove(&source, 0, 1));
    assert(original->cursors[0].boundary == 0 && original->cursors[1].boundary == 1
        && clone->cursors[0].boundary == 2 && clone->cursors[1].boundary == 3);
    discard_copy(copied);
    finish_copy();
}

/** Reused source record zero gets a different generation; copied record zero
 * starts at generation one. The removed origin must map to neither live record.
 */
static void it_never_revives_removed_origins_when_source_and_copy_record_slots_coincide(void) {
    setup_copy(false, false);
    select_origin(0, 1);
    AmaMembership removed = owner_value(0)->binding.origin;
    remove_origin(0);
    AmaListStorage source = {&world, owners[0], &owner_value(0)->items, 1, false};
    checked(ama_list_storage_insert(&source, 0, owners[1]));
    AmaMembership fresh;
    AmaHandle target;
    checked(ama_membership_at(source.list, 0, &fresh, &target));
    assert(fresh.record == removed.record && fresh.generation != removed.generation);
    yield_copy_source();
    AmaHandle copied;
    checked(ama_object_clone(&world, owners[0], true, &copied));
    OriginOwner *clone = copy_value(copied);
    assert(clone->cursors[0].boundary == 0 && clone->cursors[0].current.generation == 0
        && clone->binding.origin.generation == 0 && clone->binding.associated);
    AmaProgramAccess access = origin_access();
    AmaProgramValue value;
    checked(ama_program_select(&program, copied, &access, false, &value));
    bool refused;
    AmaStatus status = ama_program_foreach_guard(&program, &value, NULL, &refused);
    assert(status.code == AMA_STALE_REFERENCE && refused);
    assert(inspect_copy_resume(copied) == scopes[0].repeat);
    discard_copy(copied);
    finish_copy();
}

/** Obsolete origin from an exited inner role remains dead even though no active
 * cursor names that field and the replacement has the same target/position.
 */
static void it_preserves_obsolete_origins_from_an_already_exited_inner_scope(void) {
    setup_copy(false, true);
    select_origin(0, 1);
    select_origin(1, 1);
    checked(ama_program_foreach_leave(&program, owners[0], &scopes[1].plan, false));
    append_origin_reference(&owner_value(1)->other, owners[2]);
    owner_value(0)->independent = owners[1];
    const AmaProgramAccess source_receiver = {.field = 2}, other = {.field = 3};
    const AmaProgramInvocation read_source = {.target = &source_receiver, .mode = AMA_ACCESS_READ};
    bool refused;
    checked(ama_program_acquire(&program, owners[0], &read_source, &refused));
    AmaProgramValue source;
    checked(ama_program_select(&program, owners[1], &other, false, &source));
    checked(ama_program_transfer(&program, owners[0], &other, &source));
    checked(ama_program_release(&program, owners[0]));
    assert(owner_value(0)->other.count == 1 && owner_value(0)->other.incarnation != owner_value(0)->binding.origin.incarnation);
    yield_copy_source();
    AmaHandle copied;
    checked(ama_object_clone(&world, owners[0], true, &copied));
    OriginOwner *clone = copy_value(copied);
    assert(clone->cursors[0].active && !clone->cursors[1].active && clone->binding.roles == 1
        && clone->binding.origin.field == 3 && clone->binding.origin.generation == 0);
    assert(inspect_copy_resume(copied) == 0);
    AmaProgramAccess binding = origin_access();
    AmaProgramValue value;
    checked(ama_program_select(&program, copied, &binding, false, &value));
    AmaStatus status = ama_program_foreach_guard(&program, &value, NULL, &refused);
    assert(status.code == AMA_STALE_REFERENCE && refused);
    discard_copy(copied);
    finish_copy();
}

/** Ordinary copies clear control cells and positions, including a replacement
 * destination that previously held active continuation state under a stable ID.
 */
static void it_discards_control_state_in_ordinary_clones_and_stable_root_replacement(void) {
    setup_copy(true, false);
    select_origin(0, 1);
    yield_copy_source();
    AmaHandle ordinary, retained;
    checked(ama_object_clone(&world, owners[0], false, &ordinary));
    checked(ama_object_clone(&world, owners[0], true, &retained));
    OriginOwner *plain = copy_value(ordinary);
    assert(!plain->cursors[0].active && !plain->cursors[1].active && plain->binding.roles == 0
        && !plain->binding.associated && ama_handle_is_empty(plain->binding.local_value));
    AmaPoolCell *cell = NULL;
    checked(ama_program_cell(&program, ordinary, &cell));
    assert(!cell->metadata.service.resume.valid);
    uint64_t incarnation = copy_value(retained)->items.incarnation;
    checked(ama_object_replace(&world, retained, owners[0], false));
    plain = copy_value(retained);
    checked(ama_program_cell(&program, retained, &cell));
    assert(!plain->cursors[0].active && plain->binding.roles == 0 && !cell->metadata.service.resume.valid
        && plain->items.incarnation == incarnation + 1);
    checked(ama_object_replace(&world, retained, owners[0], true));
    OriginOwner *resumable = copy_value(retained);
    assert(resumable->cursors[0].active && resumable->cursors[0].incarnation == resumable->items.incarnation
        && ama_handle_equal(resumable->cursors[0].owner, retained)
        && ama_handle_equal(resumable->binding.origin.owner, retained));
    assert(inspect_copy_resume(retained) == 0);
    discard_copy(retained);
    discard_copy(ordinary);
    finish_copy();
}

/** Only an obsolete active collection rejects continuation capture before any
 * copied slot exists. Ordinary value copying may still discard that invalid state.
 */
static void it_rejects_replaced_active_collections_before_preparing_a_continuation(void) {
    setup_copy(false, false);
    select_origin(0, 1);
    const AmaProgramAccess receiver = {.field = 0}, other = {.field = 3};
    AmaProgramValue source;
    checked(ama_program_select(&program, owners[0], &other, false, &source));
    checked(ama_program_transfer(&program, owners[0], &receiver, &source));
    yield_copy_source();
    AmaProgramForeachCursor original = owner_value(0)->cursors[0];
    size_t live = owner_type.pool.live_count;
    AmaHandle copied = {0};
    AmaStatus status = ama_object_clone(&world, owners[0], true, &copied);
    assert(status.code == AMA_STALE_REFERENCE && ama_handle_is_empty(copied) && owner_type.pool.live_count == live
        && same_cursor(&original, &owner_value(0)->cursors[0]) && !world.failed);
    checked(ama_object_clone(&world, owners[0], false, &copied));
    assert(!copy_value(copied)->cursors[0].active && copy_value(copied)->binding.roles == 0);
    discard_copy(copied);
    finish_copy();
}

/** Fixed local reference cells and ordinary persistent fields remap internal
 * owned targets to the cloned tree while external list references stay unchanged.
 */
static void it_maps_active_local_handles_to_cloned_owned_targets(void) {
    setup_copy(true, false);
    static const AmaListCursorSet owned[] = {{0}, {0}, {0}, {0}, {cursor_offsets, 2}};
    owner_type.list_cursors = owned;
    scopes[0].plan.field = scopes[1].plan.field = 4;
    AmaListStorage storage = {&world, owners[0], &owner_value(0)->owned, 1, true};
    checked(ama_list_storage_insert(&storage, 0, (AmaHandle){0}));
    checked(ama_list_storage_insert(&storage, 1, (AmaHandle){0}));
    select_origin(0, 1);
    AmaHandle original = owner_value(0)->binding.local_value;
    AmaProgramAccess access = origin_access();
    AmaProgramValue selected;
    checked(ama_program_select(&program, owners[0], &access, false, &selected));
    const AmaProgramAccess independent = {.field = 2};
    checked(ama_program_transfer(&program, owners[0], &independent, &selected));
    yield_copy_source();
    AmaHandle copied;
    checked(ama_object_clone(&world, owners[0], true, &copied));
    OriginOwner *clone = copy_value(copied);
    AmaHandle internal = *ama_handle_list_element(&clone->owned, 0);
    assert(!ama_handle_equal(internal, original) && ama_handle_equal(clone->binding.local_value, internal)
        && ama_handle_equal(clone->binding.installed, internal) && ama_handle_equal(clone->cursors[0].target, internal)
        && ama_handle_equal(clone->independent, internal)
        && ama_handle_equal(*ama_handle_list_element(&clone->items, 0), owners[1]));
    assert(inspect_copy_resume(copied) == 0);
    discard_copy(copied);
    finish_copy();
}

/** Fail every preparation allocation prefix: neither original control values
 * nor the eventual successful mapped copy may lose membership/lifecycle credits.
 */
static void it_preserves_source_controls_after_every_copy_allocation_failure(void) {
    for (size_t ordinal = 1;; ordinal++) {
        setup_copy(true, false);
        select_origin(0, 1);
        yield_copy_source();
        AmaProgramForeachCursor cursor = owner_value(0)->cursors[0];
        AmaProgramForeachBinding binding = owner_value(0)->binding;
        size_t baseline = owner_type.pool.live_count;
        failure_at = allocation_count + ordinal;
        AmaHandle copied = {0};
        AmaStatus status = ama_object_clone(&world, owners[0], true, &copied);
        bool success = status.code == AMA_OK;
        assert(success || status.code == AMA_OUT_OF_MEMORY);
        assert(same_cursor(&cursor, &owner_value(0)->cursors[0])
            && same_binding(&binding, &owner_value(0)->binding));
        if (!success) { assert(ama_handle_is_empty(copied) && owner_type.pool.live_count == baseline); }
        failure_at = 0;
        if (success) { assert(inspect_copy_resume(copied) == 0); discard_copy(copied); }
        finish_copy();
        if (success) { break; }
    }
}

/** A later native clone's technical Busy cleans the earlier copied resource,
 * preserves source scopes and leaves an existing destination bank/identity intact.
 */
static void it_preserves_source_and_destination_after_a_later_native_clone_fails(void) {
    setup_native_copy();
    select_origin(0, 1);
    yield_copy_source();
    owner_value(1)->independent = owners[2];
    AmaProgramForeachCursor cursor = owner_value(0)->cursors[0];
    AmaProgramForeachBinding binding = owner_value(0)->binding;
    uint64_t incarnation = owner_value(1)->items.incarnation;
    native_fail_at = 2;
    AmaStatus status = ama_object_replace(&world, owners[1], owners[0], true);
    assert(status.code == AMA_BUSY && native_clones == 2 && native_drops == 1
        && native_type.pool.live_count == 2 && owner_type.pool.live_count == 3
        && same_cursor(&cursor, &owner_value(0)->cursors[0]) && same_binding(&binding, &owner_value(0)->binding)
        && ama_handle_equal(owner_value(1)->independent, owners[2])
        && owner_value(1)->items.incarnation == incarnation && !owner_value(1)->cursors[0].active);
    finish_native_copy();
    assert(native_drops == 3);
}

/** A root without a position can own a yielded child. Its child's cursor/local
 * handle maps an ancestor reference to the copied ancestor rather than the source.
 */
static void it_copies_resumable_owned_children_beneath_a_root_without_a_position(void) {
    setup_copy(true, false);
    AmaListStorage storage = {&world, owners[0], &owner_value(0)->owned, 1, true};
    checked(ama_list_storage_insert(&storage, 0, (AmaHandle){0}));
    AmaHandle child = *ama_handle_list_element(storage.list, 0);
    append_origin_reference(&copy_value(child)->items, owners[0]);
    AmaProgramValue target = {.type_id = 1, .form = AMA_FIELD_REFERENCE, .handle = child};
    bool refused;
    checked(ama_program_acquire_value(&program, owners[0], &target, AMA_ACCESS_ENTRY, offsetof(OriginOwner, frame), &refused));
    checked(ama_program_connect(&program, &program.current, 888, 889));
    checked(ama_program_foreach_enter(&program, child, &scopes[0].plan));
    bool selected;
    checked(ama_program_foreach_next(&program, child, &scopes[0].plan, &selected));
    assert(selected);
    AmaPoolCell *cell = NULL;
    checked(ama_program_cell(&program, child, &cell));
    cell->metadata.service.resume = (AmaResumePosition){.valid = true, .selector = 777};
    uint64_t continuation;
    checked(ama_program_yield(&program, &program.current, &continuation));
    assert(continuation == 888 && ama_handle_equal(program.current, owners[0]));
    checked(ama_program_release_result(&program, owners[0], offsetof(OriginOwner, frame), false));
    checked(ama_program_cell(&program, owners[0], &cell));
    cell->metadata.service.return_link = (AmaReturnLink){.attached = true, .host = true};
    checked(ama_program_complete(&program, &program.current, &continuation));
    assert(!cell->metadata.service.resume.valid && cell->metadata.service.activity == AMA_INSTANCE_READY);
    AmaHandle copied;
    checked(ama_object_clone(&world, owners[0], true, &copied));
    AmaHandle copied_child = *ama_handle_list_element(&copy_value(copied)->owned, 0);
    checked(ama_program_cell(&program, copied, &cell));
    assert(!cell->metadata.service.resume.valid && !copy_value(copied)->cursors[0].active);
    checked(ama_program_cell(&program, copied_child, &cell));
    assert(cell->metadata.service.resume.valid && cell->metadata.service.resume.selector == 777
        && copy_value(copied_child)->cursors[0].active
        && ama_handle_equal(copy_value(copied_child)->binding.local_value, copied)
        && ama_handle_equal(copy_value(copied_child)->cursors[0].target, copied));
    assert(inspect_copy_resume(copied_child) == 0);
    discard_copy(copied);
    finish_copy();
}

/** Separate initializer values retain separate modes even when a later recipe
 * reuses the same temporary C address. An ordinary first copy stays inactive.
 */
static void it_keeps_mixed_initializer_copy_modes_independent_of_temporary_recipe_storage(void) {
    setup_copy(true, false);
    select_origin(0, 1);
    yield_copy_source();
    program.current = owners[1];
    AmaPoolCell *caller = NULL;
    checked(ama_program_cell(&program, owners[1], &caller));
    caller->metadata.service.activity = AMA_INSTANCE_RUNNING;
    const AmaProgramAccess receiver = {.field = 4};
    AmaProgramValue destination;
    checked(ama_program_select(&program, owners[1], &receiver, false, &destination));
    const AmaRecipe children[] = {
        {.constructor = AMA_CONSTRUCT_CLONE, .source = owners[0]},
        {.constructor = AMA_CONSTRUCT_CLONE, .source = owners[0], .continuation = true}
    };
    const AmaFieldRecipe initializers = {.count = 2, .children = children, .supplied = true};
    AmaObjectOperation operation = {.world = &world};
    source_arena_init(&operation.arena, sources.allocator);
    world.operation_active = true;
    checked(ama_program_store_list(&program, &destination, NULL, &initializers, &operation));
    ama_object_unprotect(&operation);
    source_arena_destroy(&operation.arena);
    world.operation_active = false;
    AmaHandle ordinary = *ama_handle_list_element(&owner_value(1)->owned, 0);
    AmaHandle retained = *ama_handle_list_element(&owner_value(1)->owned, 1);
    AmaPoolCell *first = NULL, *second = NULL;
    checked(ama_program_cell(&program, ordinary, &first));
    checked(ama_program_cell(&program, retained, &second));
    assert(!first->metadata.service.resume.valid && !copy_value(ordinary)->cursors[0].active
        && copy_value(ordinary)->binding.roles == 0);
    assert(second->metadata.service.resume.valid && second->metadata.service.resume.selector == 777
        && copy_value(retained)->cursors[0].active && copy_value(retained)->binding.roles == 1);
    finish_copy();
}

/** The list-transfer composition initially prepares separate root boundaries;
 * its final forest pass must also redirect compiler-local sibling references.
 */
static void it_maps_control_references_after_separately_prepared_forest_roots(void) {
    setup_copy(true, false);
    AmaListStorage storage = {&world, owners[0], &owner_value(0)->owned, 1, true};
    checked(ama_list_storage_insert(&storage, 0, (AmaHandle){0}));
    checked(ama_list_storage_insert(&storage, 1, (AmaHandle){0}));
    AmaHandle children[2];
    for (size_t index = 0; index < 2; index++) {
        children[index] = *ama_handle_list_element(storage.list, index);
    }
    for (size_t index = 0; index < 2; index++) {
        AmaHandle child = children[index];
        append_origin_reference(&copy_value(child)->items, children[1 - index]);
        AmaPoolCell *cell = NULL;
        checked(ama_program_cell(&program, child, &cell));
        cell->metadata.service.activity = AMA_INSTANCE_RUNNING;
        program.current = child;
        checked(ama_program_foreach_enter(&program, child, &scopes[0].plan));
        bool selected = false;
        checked(ama_program_foreach_next(&program, child, &scopes[0].plan, &selected));
        assert(selected);
        cell->metadata.service.resume = (AmaResumePosition){.valid = true, .selector = 777};
        cell->metadata.service.return_link = (AmaReturnLink){.attached = true, .host = true};
        uint64_t continuation = UINT64_MAX;
        checked(ama_program_yield(&program, &program.current, &continuation));
        assert(continuation == 0);
    }
    program.current = owners[1];
    AmaPoolCell *caller = NULL;
    checked(ama_program_cell(&program, owners[1], &caller));
    caller->metadata.service.activity = AMA_INSTANCE_RUNNING;
    const AmaProgramAccess receiver = {.field = 4};
    AmaProgramValue destination;
    checked(ama_program_select(&program, owners[1], &receiver, false, &destination));
    const AmaProgramValue source = {.type_id = 1, .form = AMA_FIELD_OWNING_LIST,
        .list = storage.list, .continuation = true};
    AmaObjectOperation operation = {.world = &world};
    source_arena_init(&operation.arena, sources.allocator);
    world.operation_active = true;
    checked(ama_program_store_list(&program, &destination, &source, NULL, &operation));
    ama_object_unprotect(&operation);
    source_arena_destroy(&operation.arena);
    world.operation_active = false;
    for (size_t index = 0; index < 2; index++) {
        AmaHandle clone = *ama_handle_list_element(&owner_value(1)->owned, index);
        AmaHandle sibling = *ama_handle_list_element(&owner_value(1)->owned, 1 - index);
        const OriginOwner *value = copy_value(clone);
        assert(ama_handle_equal(value->binding.local_value, sibling)
            && ama_handle_equal(value->binding.installed, sibling)
            && ama_handle_equal(value->cursors[0].target, sibling)
            && ama_handle_equal(value->cursors[0].owner, clone)
            && value->cursors[0].boundary == 1 && inspect_copy_resume(clone) == 0);
    }
    finish_copy();
}

/** Run membership nonrevival, state clearing and exact-reference copy contracts. */
void run(void) {
    it_copies_live_boundaries_into_independent_namespaces_and_control_cells();
    it_never_revives_removed_origins_when_source_and_copy_record_slots_coincide();
    it_preserves_obsolete_origins_from_an_already_exited_inner_scope();
    it_discards_control_state_in_ordinary_clones_and_stable_root_replacement();
    it_rejects_replaced_active_collections_before_preparing_a_continuation();
    it_maps_active_local_handles_to_cloned_owned_targets();
    it_preserves_source_controls_after_every_copy_allocation_failure();
    it_preserves_source_and_destination_after_a_later_native_clone_fails();
    it_copies_resumable_owned_children_beneath_a_root_without_a_position();
    it_keeps_mixed_initializer_copy_modes_independent_of_temporary_recipe_storage();
    it_maps_control_references_after_separately_prepared_forest_roots();
}
/** Return only after all independent copies, original roots and pool blocks are released. */
int main(void) { run(); return EXIT_SUCCESS; }
