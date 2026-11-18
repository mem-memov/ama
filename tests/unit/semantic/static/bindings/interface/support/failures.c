/** @file
 * @brief Exercise transactional snapshot failures and maximum sequence counters with counted ownership.
 */

/** Every observed snapshot allocation fails once after accepted source setup;
 * output/publication and validated source inventories remain unchanged.
 */
static void it_unwinds_every_source_and_intrinsic_snapshot_allocation_failure(void) {
    for (size_t intrinsic = 0; intrinsic < 2; intrinsic++) {
        size_t total = 0;
        for (size_t failure = 0; failure <= total; failure++) {
            setup_binding(interface_source, STATIC_SCOPE_SOURCE_TEXT);
            BindingInterface *view = NULL;
            size_t before = allocation_count;
            if (failure != 0) { failure_at = before + failure; }
            bool success = intrinsic != 0 ? binding_list_interface(&binding_plan, SEMANTIC_LIST_INSERT, &view)
                : binding_source_interface(&binding_plan, binding_module(project.entry), BINDING_MODULE_SURFACE, NULL, 0, &view);
            if (failure == 0) { assert(success); total = allocation_count - before; assert(total != 0); }
            else {
                assert(!success && view == NULL && binding_plan.first_interface == NULL && binding_plan.last_interface == NULL);
                assert(binding_plan.status == BINDING_OUT_OF_MEMORY && diagnostic_valid(&binding_plan.error));
                assert(!binding_invalid(&binding_plan, (SourceSpan){0}, "later source error"));
                assert(binding_plan.status == BINDING_OUT_OF_MEMORY);
            }
            destroy_binding();
        }
    }
}

/** The maximum counter refuses without allocation/linking, and sticky failure
 * cannot be replaced by a second error or allocate additional scratch.
 */
static void it_checks_slot_count_overflow_and_first_failure_without_publication(void) {
    setup_binding(interface_source, STATIC_SCOPE_SOURCE_TEXT);
    SourceSpan site = static_name_site(project.entry, "shared");
    BindingSlot prototype = {.name = {source_bytes(site.source) + site.start, site.end - site.start, site}};
    BindingSequence sequence = {.count = SIZE_MAX};
    size_t before = allocation_count;
    assert(!binding_slot_append(&binding_plan, &sequence, prototype));
    assert(binding_plan.status == BINDING_OUT_OF_MEMORY && allocation_count == before);
    assert(sequence.first == NULL && sequence.last == NULL && sequence.count == SIZE_MAX);
    assert(!binding_invalid(&binding_plan, site, "later error"));
    assert(binding_allocate(&binding_plan, 1) == NULL && allocation_count == before);
    destroy_binding();
}

/** Refuse each allocation after one successful publication. A second failing
 * construction cannot modify that interface, its tail link, or caller output.
 */
static void it_preserves_published_snapshots_when_later_construction_fails(void) {
    size_t total = 0;
    for (size_t failure = 0; failure <= total; failure++) {
        setup_binding(interface_source, STATIC_SCOPE_SOURCE_TEXT);
        const StaticModule *module = binding_module(project.entry);
        BindingInterface *first = NULL;
        assert(binding_source_interface(&binding_plan, module, BINDING_MODULE_SURFACE, NULL, 0, &first));
        const BindingSlot *slot = first->sequences[BINDING_STATIC_INPUT].first;
        BindingInterface *output = first;
        size_t before = allocation_count;
        if (failure != 0) { failure_at = before + failure; }
        bool success = binding_source_interface(&binding_plan, module, BINDING_MODULE_SURFACE, NULL, 0, &output);
        if (failure == 0) { assert(success && output != first); total = allocation_count - before; }
        else {
            assert(!success && output == first && binding_plan.first_interface == first && binding_plan.last_interface == first);
            assert(first->next == NULL && first->sequences[BINDING_STATIC_INPUT].first == slot);
            assert(first->sequences[BINDING_STATIC_INPUT].count == 4 && slot->original_ordinal == 0);
            assert(binding_plan.status == BINDING_OUT_OF_MEMORY);
        }
        destroy_binding();
    }
}
