/** @file
 * @brief Reject damaged logical layouts and release every partial fixed-storage allocation.
 */
/** The independent storage boundary detects wrong counts, domains, parent links, cells, origin sharing and layout order. */
static void it_rejects_incomplete_or_duplicated_logical_storage_without_allocating(void) {
    for (size_t shape = 0; shape < 14; shape++) {
        prepare_storage(storage_main);
        bool built = plan_storage(&plan);
        assert(built);
        switch (shape) {
            case 0: plan.context_storage.pools++; break;
            case 1: plan.context_storage.constants++; break;
            case 2: plan.context_storage.domains = 0; break;
            case 3: plan.entry->stable_slots = false; break;
            case 4: plan.entry->service_storage &= ~STORAGE_CONSTRUCTING_IDENTITY; break;
            case 5: frame_at(2)->operation_parent = NULL; break;
            case 6: frame_at(3)->traversal_parent = NULL; break;
            case 7: frame_at(3)->cell = frame_at(4)->cell; break;
            case 8: frame_at(3)->origin = frame_at(4)->origin; break;
            case 9: plan.entry->cells->slot++; break;
            case 10: plan.entry->frame_count++; break;
            case 11: plan.entry->layout_ordinal++; break;
            case 12: plan.shared_first->service_storage = STORAGE_BUSY; break;
            case 13: plan.entry = plan.entry->first->type; break;
        }
        size_t before = allocation_count;
        bool valid = plan_validate_storage(&plan);
        assert(!valid && plan.status == PLAN_INVALID && allocation_count == before);
        destroy_plan();
    }
}
/** Fail each allocation while keeping borrowed semantic state live and checking the plan's whole owned prefix is released. */
static void it_unwinds_every_storage_frame_cell_and_origin_allocation(void) {
    prepare_storage(storage_main);
    size_t before = allocation_count;
    size_t live_before = live_allocations;
    bool built = plan_storage(&plan);
    assert(built);
    size_t count = allocation_count - before;
    program_plan_destroy(&plan);
    assert(live_allocations == live_before);
    for (size_t failure = 1; failure <= count; failure++) {
        program_plan_init(&plan, &semantic, sources.allocator);
        failure_at = allocation_count + failure;
        built = plan_storage(&plan);
        assert(!built && !plan.storage_ready && plan.status == PLAN_OUT_OF_MEMORY);
        assert(diagnostic_valid(&plan.error));
        program_plan_destroy(&plan);
        assert(live_allocations == live_before);
        failure_at = 0;
    }
    destroy_semantic();
}
