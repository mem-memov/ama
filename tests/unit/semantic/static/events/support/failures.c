/** @file
 * @brief Check occurrence allocation refusal, first error and maximum-count ownership.
 */
/** Fail each actual collection allocation, then check the independent count
 * overflow path. Every partial occurrence/frame remains arena-owned.
 */
static void it_unwinds_every_occurrence_collection_allocation_failure_and_checked_count(void) {
    const char *scenarios[] = {event_source, graph_event_source, expanded_event_source};
    const char *targets[] = {"@...\n.selected export\n.selected type\n",
        "@...\n.out\n.slot\n", STATIC_SCOPE_SOURCE_TEXT};
    for (size_t scenario = 0; scenario < sizeof(scenarios) / sizeof(*scenarios); scenario++) {
        size_t total = 0;
        for (size_t failure = 0; failure <= total; failure++) {
            setup_static(scenarios[scenario], targets[scenario]);
            StaticModule *module = static_module_add(&inventory, project.entry);
            assert(module != NULL);
            size_t before = allocation_count;
            if (failure != 0) { failure_at = before + failure; }
            bool collected = static_collect_occurrences(&inventory, module);
            if (failure == 0) { assert(collected); total = allocation_count - before; assert(total != 0); }
            else { assert(!collected && inventory.status == STATIC_OUT_OF_MEMORY && diagnostic_valid(&inventory.error)); }
            destroy_static();
        }
    }
    setup_static(STATIC_SCOPE_SOURCE_TEXT, STATIC_SCOPE_SOURCE_TEXT);
    StaticModule *module = static_module_add(&inventory, project.entry);
    assert(module != NULL);
    module->occurrence_count = SIZE_MAX;
    size_t before = allocation_count;
    assert(!static_occurrence_append(&inventory, module, (StaticOccurrence){0}));
    assert(allocation_count == before && inventory.status == STATIC_OUT_OF_MEMORY && module->first_occurrence == NULL);
    destroy_static();
}
