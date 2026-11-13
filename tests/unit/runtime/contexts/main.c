/**
 * @file
 * @brief Verify isolated context ownership, identity retirement and nonreentrant host admission.
 *
 * Reuse counted address-sensitive native resources, while context factories own independent
 * descriptors/constants. Thread cases perform no concurrent fixture allocator/native-hook work.
 */
#include "common.c"
#include "support/order.c"
#include "support/diagnostics.c"
#include "support/callbacks.c"
#include "support/concurrency.c"

/** Prove factory ownership is per context and pool capacity does not imply a default instance. */
static void it_owns_constants_pools_and_diagnostics_without_an_implicit_root(void) {
    setup_contexts();
    uint64_t first = create_context(), second = create_context();
    assert(first != second && constructed == 2);
    AmaContextLease left = {0}, right = {0};
    AmaContextView a = enter_context(first, &left), b = enter_context(second, &right);
    assert(a.world != b.world && a.program != b.program && a.control != b.control);
    assert(a.world->types != b.world->types && a.world->types[0] != b.world->types[0]);
    assert(a.program->constants != b.program->constants);
    assert(a.program->constants[0].storage != b.program->constants[0].storage);
    const AmaConstantValue *ac = a.program->constants[0].storage->items;
    const AmaConstantValue *bc = b.program->constants[0].storage->items;
    assert(ac != bc && ac->payload != bc->payload);
    const NativeValue *av = ac->payload, *bv = bc->payload;
    assert(av->number == 37 && bv->number == 37 && av->resource != bv->resource);
    for (size_t index = 0; index < 2; index++) {
        assert(a.world->types[index]->pool.live_count == 0);
        assert(b.world->types[index]->pool.live_count == 0);
        assert(a.world->types[index]->pool.allocated_blocks == 0);
    }
    assert(ama_handle_is_empty(a.control->root) && ama_handle_is_empty(b.control->root));
    assert(a.status.code == AMA_OK && b.status.code == AMA_OK);
    a.program->initialization_operation = 91;
    assert(b.program->initialization_operation != 91);
    leave_context(&right);
    leave_context(&left);
    AmaStatus status = ama_context_dispose(&registry, first);
    assert(status.code == AMA_OK && destroyed == 1);
    b = enter_context(second, &right);
    assert(((const NativeValue *)b.program->constants[0].storage->items->payload)->number == 37);
    leave_context(&right);
    finish_contexts();
}

/** Equal local triples must not override host origin; rejection leaves output and Ama history intact. */
static void it_rejects_foreign_origin_before_touching_colliding_local_handles(void) {
    setup_contexts();
    uint64_t first = create_context(), second = create_context();
    AmaContextLease left = {0}, right = {0};
    AmaContextView a = enter_context(first, &left), b = enter_context(second, &right);
    AmaContextReference ar = create_root(&left, a), br = create_root(&right, b);
    assert(ama_handle_equal(ar.local, br.local) && ar.context != br.context);
    AmaObjectType *type = NULL;
    AmaPoolCell *cell = NULL;
    AmaStatus status = ama_context_resolve(&right, br, AMA_ACCESS_READ, &type, &cell);
    assert(status.code == AMA_OK);
    AmaObjectType *unchanged_type = type;
    AmaPoolCell *unchanged_cell = cell;
    cell->metadata.service.last_refusal = ama_status_make(AMA_BUSY, NULL, 0);
    status = ama_context_resolve(&right, ar, AMA_ACCESS_READ, &type, &cell);
    assert(status.code == AMA_TYPE_MISMATCH && type == unchanged_type && cell == unchanged_cell);
    status = ama_context_resolve(&right, (AmaContextReference){.context = first}, AMA_ACCESS_READ, &type, &cell);
    assert(status.code == AMA_TYPE_MISMATCH && cell == unchanged_cell);
    assert(cell->metadata.service.last_refusal.code == AMA_BUSY && !b.world->failed);
    status = ama_context_resolve(&left, ar, AMA_ACCESS_READ, &type, &cell);
    assert(status.code == AMA_OK);
    cell->metadata.service.activity = AMA_INSTANCE_RUNNING;
    AmaContextReference running = {0};
    status = ama_context_reference(&left, ar.local, &running);
    assert(status.code == AMA_OK && running.context == first);
    status = ama_context_resolve(&left, running, AMA_ACCESS_READ, &type, &cell);
    assert(status.code == AMA_BUSY);
    cell->metadata.service.activity = AMA_INSTANCE_READY;
    leave_context(&right);
    leave_context(&left);
    finish_contexts();
}

/** Terminal context cleanup owns every root, then reverse constant storage, without hidden root defaults. */
static void it_destroys_all_explicit_roots_and_invalidates_the_context_before_reuse(void) {
    setup_contexts();
    uint64_t first = create_context();
    AmaContextLease lease = {0};
    AmaContextView view = enter_context(first, &lease);
    AmaContextReference one = create_root(&lease, view), two = create_root(&lease, view);
    root_value(&lease, one)->self = two.local;
    assert(view.world->types[1]->pool.live_count == 2 && constructed == 7);
    leave_context(&lease);
    AmaStatus status = ama_context_dispose(&registry, first);
    assert(status.code == AMA_OK && destroyed == 7 && live_allocations == 0);
    const size_t expected[] = {7, 6, 5, 4, 3, 2, 37};
    assert(memcmp(destruction_order, expected, sizeof(expected)) == 0);
    status = ama_context_enter(&registry, first, &lease);
    assert(status.code == AMA_STALE_REFERENCE && lease.context == NULL);
    status = ama_context_registry_destroy(&registry);
    assert(status.code == AMA_OK);
    status = ama_context_registry_init(&registry, sources.allocator);
    assert(status.code == AMA_OK);
    uint64_t next = create_context();
    assert(next != first);
    status = ama_context_enter(&registry, first, &lease);
    assert(status.code == AMA_STALE_REFERENCE);
    finish_contexts();
}

/** Close preflight must restore every earlier gate when a later context remains leased. */
static void it_refuses_reentry_and_rolls_back_busy_registry_close_admission(void) {
    setup_contexts();
    uint64_t tail = create_context(), middle = create_context(), head = create_context();
    uint64_t existing = tail;
    size_t constructors_before = constructed;
    AmaStatus rejected = ama_context_create(&registry, &definition, &existing);
    assert(rejected.code == AMA_INVALID_STATE && existing == tail && constructed == constructors_before);
    AmaContextLease held = {0}, other = {0}, duplicate = {0};
    (void)enter_context(tail, &held);
    size_t before = destroyed;
    AmaStatus status = ama_context_enter(&registry, tail, &duplicate);
    assert(status.code == AMA_BUSY && duplicate.context == NULL);
    status = ama_context_dispose(&registry, tail);
    assert(status.code == AMA_BUSY && destroyed == before);
    status = ama_context_registry_close(&registry);
    assert(status.code == AMA_BUSY && !registry.closed && destroyed == before);
    (void)enter_context(head, &other);
    leave_context(&other);
    (void)enter_context(middle, &other);
    leave_context(&other);
    leave_context(&held);
    status = ama_context_registry_close(&registry);
    assert(status.code == AMA_OK && destroyed == 3);
    status = ama_context_registry_close(&registry);
    assert(status.code == AMA_OK && destroyed == 3);
    uint64_t absent = 0;
    status = ama_context_create(&registry, &definition, &absent);
    assert(status.code == AMA_INVALID_STATE && absent == 0);
    finish_contexts();
}

/** Accepted registry closure must consume every context even when a native destructor fails. */
static void it_finishes_all_registry_cleanup_and_preserves_the_first_destructor_error(void) {
    setup_contexts();
    uint64_t first = create_context(), second = create_context();
    failed_destructor = 1;
    AmaStatus status = ama_context_registry_close(&registry);
    assert(status.code == AMA_IO_FAILURE && destroyed == 2 && reports == 1);
    assert(registry.closed && registry.pending == 0 && registry.first == NULL && live_allocations == 0);
    AmaContextLease lease = {0};
    status = ama_context_enter(&registry, first, &lease);
    assert(status.code == AMA_STALE_REFERENCE);
    status = ama_context_enter(&registry, second, &lease);
    assert(status.code == AMA_STALE_REFERENCE);
    status = ama_context_registry_close(&registry);
    assert(status.code == AMA_IO_FAILURE && destroyed == 2);
    status = ama_context_registry_destroy(&registry);
    assert(status.code == AMA_IO_FAILURE && !registry.initialized);
    destroy_objects();
}

/** Exhaust the same exact identity algorithm with a reduced domain and UINT64_MAX boundary. */
static void it_never_wraps_or_reissues_exhausted_context_identity_frontiers(void) {
    _Atomic uint64_t clock = 0;
    uint64_t issued = 0;
    for (uint64_t expected = 1; expected <= 3; expected++) {
        AmaStatus status = ama_context_identity_next(&clock, 3, &issued);
        assert(status.code == AMA_OK && issued == expected);
    }
    issued = 91;
    AmaStatus status = ama_context_identity_next(&clock, 3, &issued);
    assert(status.code == AMA_LIMIT_EXCEEDED && issued == 91 && atomic_load(&clock) == 3);
    _Atomic uint64_t full = UINT64_MAX - 1;
    status = ama_context_identity_next(&full, UINT64_MAX, &issued);
    assert(status.code == AMA_OK && issued == UINT64_MAX);
    issued = 92;
    status = ama_context_identity_next(&full, UINT64_MAX, &issued);
    assert(status.code == AMA_LIMIT_EXCEEDED && issued == 92 && atomic_load(&full) == UINT64_MAX);
    status = ama_context_identity_next(&clock, 0, &issued);
    assert(status.code == AMA_INVALID_STATE && issued == 92);
}

/** Fail every measured allocator call in the actual private factory, never publish a prefix. */
static void it_unwinds_every_factory_allocation_failure_without_poisoning_the_registry(void) {
    setup_contexts();
    (void)create_context();
    size_t allocations = allocation_count;
    finish_contexts();
    for (size_t failure = 1; failure <= allocations; failure++) {
        setup_contexts();
        failure_at = allocation_count + failure;
        uint64_t identity = 0;
        AmaStatus status = ama_context_create(&registry, &definition, &identity);
        assert(status.code == AMA_OUT_OF_MEMORY && identity == 0);
        assert(registry.first == NULL && registry.pending == 0 && live_allocations == 0);
        failure_at = 0;
        (void)create_context();
        finish_contexts();
    }
}

/** Invalid shapes must fail before constructors; constructor failure preserves its primary cleanup error. */
static void it_rejects_bad_schema_and_cleans_failed_constants_without_a_published_handle(void) {
    setup_contexts();
    AmaContextType invalid_types[2] = {context_types[0], context_types[1]};
    AmaContextDefinition invalid = definition;
    invalid.types = invalid_types;
    invalid.type_count = SIZE_MAX;
    uint64_t overflow_identity = 0;
    AmaStatus overflow = ama_context_create(&registry, &invalid, &overflow_identity);
    assert(overflow.code == AMA_LIMIT_EXCEEDED && overflow_identity == 0 && allocation_count == 0);
    invalid.type_count = 2;
    invalid_types[0].field_count = invalid_types[1].field_count = 0;
    invalid_types[1].type_id = 1;
    uint64_t identity = 0;
    AmaStatus status = ama_context_create(&registry, &invalid, &identity);
    assert(status.code == AMA_TYPE_MISMATCH && constructed == 0 && allocation_count == 0);
    invalid_types[0] = context_types[0];
    invalid_types[1] = context_types[1];
    invalid_types[1].native_offset = invalid_types[1].size + 1;
    status = ama_context_create(&registry, &invalid, &identity);
    assert(status.code == AMA_INVALID_STATE && identity == 0 && constructed == 0 && live_allocations == 0);
    AmaContextConstant constants[2] = {context_constant, context_constant};
    invalid = definition;
    invalid.constants = constants;
    invalid.constant_count = 2;
    failed_constructor = 2;
    failed_destructor = 1;
    status = ama_context_create(&registry, &invalid, &identity);
    assert(status.code == AMA_INVALID_LITERAL && identity == 0);
    assert(constructed == 2 && destroyed == 1 && reports == 2 && live_allocations == 0);
    failed_constructor = failed_destructor = 0;
    (void)create_context();
    finish_contexts();
}

/** Only the accepted failed context becomes terminal; another context retains healthy storage. */
static void it_keeps_failure_and_diagnostic_state_local_to_the_failed_context(void) {
    setup_contexts();
    uint64_t first = create_context(), second = create_context();
    AmaContextLease left = {0}, right = {0};
    AmaContextView a = enter_context(first, &left), b = enter_context(second, &right);
    AmaContextReference ar = create_root(&left, a), br = create_root(&right, b);
    failed_destructor = destroyed + 1;
    AmaStatus status = ama_object_destroy(a.world, ar.local);
    assert(status.code == AMA_IO_FAILURE && a.world->failed && !b.world->failed);
    status = ama_context_inspect(&left, &a);
    assert(status.code == AMA_OK && a.status.code == AMA_IO_FAILURE);
    assert(a.control->phase == AMA_RUNTIME_TERMINATED);
    AmaContextReference unchanged = br;
    status = ama_context_reference(&left, ar.local, &unchanged);
    assert(status.code == AMA_INVALID_STATE && unchanged.context == br.context);
    status = ama_context_inspect(&right, &b);
    assert(status.code == AMA_OK && b.status.code == AMA_OK);
    assert(root_value(&right, br)->native.resource != NULL);
    leave_context(&right);
    leave_context(&left);
    status = ama_context_dispose(&registry, first);
    assert(status.code == AMA_IO_FAILURE);
    finish_contexts();
}

/** Malformed transport becomes context-local technical failure without allocation.
 * Its fallback bytes outlive context disposal and never overwrite the instance's refusal.
 */
static void it_normalizes_malformed_context_reports_without_allocating_or_changing_refusals(void) {
    setup_contexts();
    uint64_t identity = create_context();
    AmaContextLease lease = {0};
    AmaContextView view = enter_context(identity, &lease);
    (void)create_root(&lease, view);
    AmaPoolCell *root = view.world->last_root;
    root->metadata.service.last_refusal = ama_status_make(AMA_BUSY, NULL, 0);
    size_t before = allocation_count;
    view.world->report(view.world->report_context, ama_status_make(AMA_OK, NULL, 1));
    AmaStatus status = ama_context_inspect(&lease, &view);
    assert(status.code == AMA_OK && allocation_count == before && reports == 1);
    assert(view.status.code == AMA_INVALID_STATE && view.world->failed);
    assert(view.control->phase == AMA_RUNTIME_TERMINATED);
    assert(root->metadata.service.last_refusal.code == AMA_BUSY);
    const char *message = view.status.message;
    size_t length = view.status.message_length;
    assert(message != NULL && length == sizeof("Malformed native status") - 1);
    leave_context(&lease);
    status = ama_context_dispose(&registry, identity);
    assert(status.code == AMA_INVALID_STATE && status.message == message);
    assert(memcmp(message, "Malformed native status", length) == 0);
    finish_contexts();
}

/** Remove public identity before callbacks and pin the registry until native cleanup is finished. */
static void it_invalidates_before_cleanup_and_keeps_lifecycle_callbacks_outside_metadata_gates(void) {
    setup_contexts();
    ContextCleanupProbe probe = {0};
    definition.report = context_cleanup_report;
    definition.report_context = &probe;
    probe.retiring = create_context();
    probe.independent = create_context();
    failed_destructor = destroyed + 1;
    AmaStatus status = ama_context_dispose(&registry, probe.retiring);
    assert(status.code == AMA_IO_FAILURE && probe.observed == 1 && registry.pending == 0);
    assert(!registry.closed && registry.initialized);
    finish_contexts();
}

/** Simultaneous factories must publish every record with a distinct nonzero checked identity. */
static void it_publishes_distinct_context_identities_from_simultaneous_factories(void) {
    AmaContextRegistry owner;
    AmaStatus status = ama_context_registry_init(&owner, source_system_allocator());
    assert(status.code == AMA_OK);
    ContextFactoryThread left = {.registry = &owner}, right = {.registry = &owner};
    thrd_t first, second;
    int result = thrd_create(&first, context_factory_worker, &left);
    assert(result == thrd_success);
    result = thrd_create(&second, context_factory_worker, &right);
    assert(result == thrd_success);
    int child = -1;
    result = thrd_join(first, &child);
    assert(result == thrd_success && child == 0);
    result = thrd_join(second, &child);
    assert(result == thrd_success && child == 0);
    for (size_t index = 0; index < 32; index++) {
        for (size_t other = 0; other < 32; other++) {
            assert(left.identities[index] != right.identities[other]);
            if (index != other) { assert(left.identities[index] != left.identities[other]); }
            if (index != other) { assert(right.identities[index] != right.identities[other]); }
        }
    }
    size_t contexts = 0;
    for (const AmaContext *context = owner.first; context != NULL; context = context->next) { contexts++; }
    assert(contexts == 64 && owner.pending == 0);
    status = ama_context_registry_destroy(&owner);
    assert(status.code == AMA_OK);
}

/** Join synchronizes observations; actual independent context admission needs no global execution owner. */
static void it_rejects_a_simultaneous_executor_but_allows_an_independent_context(void) {
    setup_contexts();
    uint64_t first = create_context(), second = create_context();
    AmaContextLease left = {0}, right = {0};
    AmaContextView a = enter_context(first, &left), b = enter_context(second, &right);
    AmaContextReference ar = create_root(&left, a), br = create_root(&right, b);
    size_t original = root_value(&left, ar)->native.number;
    leave_context(&right);
    ContextThread input = {.busy = first, .independent = second, .reference = br};
    thrd_t worker;
    int result = thrd_create(&worker, context_worker, &input);
    assert(result == thrd_success);
    int worker_result = -1;
    result = thrd_join(worker, &worker_result);
    assert(result == thrd_success && worker_result == 0 && input.completed);
    assert(root_value(&left, ar)->native.number == original);
    leave_context(&left);
    (void)enter_context(second, &right);
    assert(root_value(&right, br)->native.number == 900);
    leave_context(&right);
    finish_contexts();
}

/** Run the context ownership, checked-identity, failure-prefix and concurrency contracts. */
void run(void) {
    it_owns_constants_pools_and_diagnostics_without_an_implicit_root();
    it_rejects_foreign_origin_before_touching_colliding_local_handles();
    it_destroys_all_explicit_roots_and_invalidates_the_context_before_reuse();
    it_orders_mixed_root_types_after_early_disposal_and_slot_reuse();
    it_keeps_replaced_root_identity_in_its_original_cleanup_position();
    it_releases_a_retained_root_slot_after_destructive_replacement_failure();
    it_registers_public_clones_after_their_source_roots();
    it_excludes_failed_create_and_clone_from_root_publication_order();
    it_continues_reverse_root_cleanup_after_distinct_destructor_errors();
    it_refuses_reentry_and_rolls_back_busy_registry_close_admission();
    it_finishes_all_registry_cleanup_and_preserves_the_first_destructor_error();
    it_never_wraps_or_reissues_exhausted_context_identity_frontiers();
    it_unwinds_every_factory_allocation_failure_without_poisoning_the_registry();
    it_rejects_bad_schema_and_cleans_failed_constants_without_a_published_handle();
    it_keeps_failure_and_diagnostic_state_local_to_the_failed_context();
    it_normalizes_malformed_context_reports_without_allocating_or_changing_refusals();
    it_streams_checked_diagnostics_without_runtime_allocation();
    it_invalidates_before_cleanup_and_keeps_lifecycle_callbacks_outside_metadata_gates();
    it_publishes_distinct_context_identities_from_simultaneous_factories();
    it_rejects_a_simultaneous_executor_but_allows_an_independent_context();
}

/** Return success only after every explicit context contract assertion completes. */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
