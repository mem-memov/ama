/** @file
 * @brief Exhaust the source/test/standard placement matrix over real captured modules.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
/** Root/area equality is the independent oracle for all nine transitions.
 * Physical spelling never overrides logical namespace; failures allocate nothing,
 * retain a diagnostic requester, preserve output and poison subsequent sealing.
 */
static void it_preserves_namespace_for_every_source_test_standard_pair(void) {
    const char *const originals[] = {"/vendor/pkg/v1.0.0/src/parentish",
        "/vendor/pkg/v1.0.0/tests/check", "//child"};
    const char *const destinations[] = {"/other/pkg/v9.0.0/src/copied",
        "/other/pkg/v9.0.0/tests/copied", "//copied"};
    for (size_t from = 0; from < 3; from++) {
        for (size_t to = 0; to < 3; to++) {
            setup_test(); selection_fixtures();
            file("generated/project/packages/vendor/pkg/v1.0.0/tests/check", "@!\n");
            selection_capture_scope(true, true);
            source_clone_map_init(&map, sources.allocator, &capture);
            const ModuleSource *source = original(originals[from]);
            SourceCloneMapping sentinel = {0};
            const SourceCloneMapping *row = &sentinel;
            size_t before = allocation_count;
            SourceCloneStatus status = source_clone_map_add(&map, source, "/prepared/copied", destinations[to], &row);
            if (from == to) {
                assert(status == SOURCE_CLONE_OK && row != &sentinel);
                assert(source_clone_map_seal(&map) == SOURCE_CLONE_OK);
            } else {
                assert(status == SOURCE_CLONE_NAMESPACE && row == &sentinel && !map.first);
                assert(map.requester == source && !map.conflict && allocation_count == before);
                assert(source_clone_map_seal(&map) == SOURCE_CLONE_NAMESPACE && !map.ready);
                assert(source_clone_map_add(&map, source, "/prepared/copied", destinations[from], &row) == SOURCE_CLONE_NAMESPACE);
            }
            source_clone_map_destroy(&map); finish();
            assert(unlink("generated/project/packages/vendor/pkg/v1.0.0/tests/check") == 0);
        }
    }
}
