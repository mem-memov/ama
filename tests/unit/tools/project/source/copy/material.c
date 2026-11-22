/** @file
 * @brief Verify exact material joins independently of catalog metadata and publication.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
/** Explicit source records keep malformed opaque bytes separate from transformed code. */
static const SourceText *material_text(const char *path, const unsigned char *bytes, size_t length) {
    const SourceText *text = NULL;
    assert(source_manager_add(&sources, path, bytes, length, &text) == SOURCE_OK); return text;
}
/** Complete raw membership, alternate equal-byte source ownership, transformed
 * code and opaque/empty entries produce one exact row per sealed P member. */
static void it_joins_every_mapped_member_with_exact_prepared_or_opaque_content(void) {
    setup_test();
    const unsigned char opaque_bytes[] = {0, 255, '\r', '\n'};
    const SourceText *original = material_text("/original/code", (const unsigned char *)"old", 3);
    const SourceText *separate = material_text("/original/code", (const unsigned char *)"old", 3);
    const SourceText *opaque = material_text("/original/data", opaque_bytes, sizeof(opaque_bytes));
    const SourceText *empty = material_text("/original/empty", (const unsigned char *)"", 0);
    assert(original != separate);
    SourceFileMap map; source_file_map_init(&map, sources.allocator); const SourceFileMapping *row = NULL;
    assert(source_file_map_observation(&map, "/original/code", original, false, "/final/code", &row) == SOURCE_FILE_MAP_OK);
    assert(source_file_map_observation(&map, "/original/data", opaque, false, "/final/data", &row) == SOURCE_FILE_MAP_OK);
    assert(source_file_map_observation(&map, "/original/empty", empty, false, "/final/empty", &row) == SOURCE_FILE_MAP_OK);
    assert(source_file_map_observation(&map, "/original/directory", NULL, true, "/final/directory", &row) == SOURCE_FILE_MAP_OK);
    assert(source_file_map_seal(&map) == SOURCE_FILE_MAP_OK);
    SourceTextEdits edits; source_text_edits_init(&edits, sources.allocator, separate);
    assert(source_text_edits_add(&edits, (SourceSpan){separate, 0, 3}, (const unsigned char *)"new", 3) == SOURCE_EDITS_OK);
    const SourceText *result = NULL;
    assert(source_text_edits_apply(&edits, &sources, "/final/code", &result) == SOURCE_EDITS_OK);
    SourceGraphPreparedText prepared = {NULL, &edits};
    SourceArena arena; source_arena_init(&arena, sources.allocator);
    SourceCopyMaterial output = {0}; SourceMaterialIssue issue;
    assert(source_copy_material_prepare(&arena, &map, &prepared, &output, &issue) == SOURCE_MATERIAL_OK);
    assert(output.count == 4 && !issue.mapping && !issue.prepared);
    const SourceFileMapping *mapping = map.first;
    for (size_t i = 0; i < output.count; i++, mapping = mapping->next) {
        assert(output.entries[i].mapping == mapping);
        assert(output.entries[i].content == (!strcmp(mapping->destination, "/final/code") ? result : mapping->original->file.content));
    }
    size_t baseline = live_allocations, total = 0;
    for (size_t failure = 0; !failure || failure <= total; failure++) {
        SourceArena attempt; source_arena_init(&attempt, sources.allocator); SourceCopyMaterial candidate = output;
        size_t start = allocation_count; if (failure) { failure_at = start + failure; }
        SourceMaterialStatus status = source_copy_material_prepare(&attempt, &map, &prepared, &candidate, &issue);
        if (!failure) { assert(status == SOURCE_MATERIAL_OK); total = allocation_count - start; assert(total); }
        else { assert(status == SOURCE_MATERIAL_MEMORY && candidate.entries == output.entries && candidate.count == output.count); }
        failure_at = 0; source_arena_destroy(&attempt); assert(live_allocations == baseline);
    }
    assert(source_copy_material_prepare(&arena, &map, NULL, &output, &issue) == SOURCE_MATERIAL_OK);
    assert(output.count == 4);
    for (size_t i = 0; i < output.count; i++) { assert(output.entries[i].content == output.entries[i].mapping->original->file.content); }
    source_arena_destroy(&arena); source_text_edits_destroy(&edits); source_file_map_destroy(&map);
    SourceFileMap zero; source_file_map_init(&zero, sources.allocator);
    assert(source_file_map_seal(&zero) == SOURCE_FILE_MAP_OK);
    source_arena_init(&arena, sources.allocator); size_t calls = allocation_count;
    assert(source_copy_material_prepare(&arena, &zero, NULL, &output, &issue) == SOURCE_MATERIAL_OK);
    assert(!output.count && !output.entries && allocation_count == calls);
    source_arena_destroy(&arena); source_file_map_destroy(&zero); destroy_test();
}
/** Unmapped/directory results, wrong original paths/bytes, duplicates and
 * unfinished edits refuse without publishing a result or changing captured bytes. */
static void it_refuses_unrelated_ambiguous_and_unapplied_material_preparations(void) {
    setup_test(); const SourceText *original = material_text("/original/code", (const unsigned char *)"old", 3);
    SourceFileMap map; source_file_map_init(&map, sources.allocator); const SourceFileMapping *row = NULL;
    assert(source_file_map_observation(&map, "/original/code", original, false, "/final/code", &row) == SOURCE_FILE_MAP_OK);
    assert(source_file_map_observation(&map, "/original/directory", NULL, true, "/final/directory", &row) == SOURCE_FILE_MAP_OK);
    assert(source_file_map_seal(&map) == SOURCE_FILE_MAP_OK);
    const SourceMaterialStatus expected[] = {SOURCE_MATERIAL_UNMAPPED, SOURCE_MATERIAL_UNMAPPED,
        SOURCE_MATERIAL_ORIGINAL, SOURCE_MATERIAL_ORIGINAL, SOURCE_MATERIAL_DUPLICATE, SOURCE_MATERIAL_INVALID};
    SourceArena arena; source_arena_init(&arena, sources.allocator);
    SourceCopyMaterial sentinel = {(const SourceCopyMaterialEntry *)&map, 7};
    for (unsigned profile = 0; profile < sizeof(expected) / sizeof(*expected); profile++) {
        const SourceText *input = profile == 2 ? material_text("/unrelated/code", (const unsigned char *)"old", 3)
            : profile == 3 ? material_text("/original/code", (const unsigned char *)"bad", 3) : original;
        SourceTextEdits edits; source_text_edits_init(&edits, sources.allocator, input);
        const char *target = profile == 0 ? "/unmapped/code" : profile == 1 ? "/final/directory" : "/final/code";
        const SourceText *result = NULL;
        if (profile != 5) { assert(source_text_edits_apply(&edits, &sources, target, &result) == SOURCE_EDITS_OK); }
        SourceGraphPreparedText second = {NULL, &edits}, first = {profile == 4 ? &second : NULL, &edits};
        SourceCopyMaterial output = sentinel; SourceMaterialIssue issue;
        assert(source_copy_material_prepare(&arena, &map, &first, &output, &issue) == expected[profile]);
        assert(output.entries == sentinel.entries && output.count == sentinel.count && issue.prepared);
        source_text_edits_destroy(&edits);
    }
    SourceFileMap open; source_file_map_init(&open, sources.allocator); SourceCopyMaterial output = sentinel; SourceMaterialIssue issue;
    assert(source_copy_material_prepare(&arena, &open, NULL, &output, &issue) == SOURCE_MATERIAL_INVALID);
    assert(output.entries == sentinel.entries && output.count == sentinel.count);
    source_file_map_destroy(&open); source_arena_destroy(&arena); source_file_map_destroy(&map); destroy_test();
}
