/** @file
 * @brief Prepare finite real-project copies inside one unchanged physical observation envelope.
 * @see [T275 contract](../../../../../../../docs/implementation/source-operations.md#t275-entry-review-and-execution)
 */
/** Test placement is one explicitly selected directory-relative mapping. */
static const char *domain_mapped_path(DomainFixture *fixture, const char *old) {
    size_t length = strlen(fixture->prefix);
    assert(source_image_below(old, fixture->prefix));
    if (!old[length]) { return fixture->destination; }
    return domain_path(fixture, fixture->destination, old + length + 1);
}
/** Every source directory/file is fully observed; assign fresh distinct virtual
 * classes, retain original rows, and select final roots independently. */
static void domain_project(DomainFixture *fixture) {
    const SourceCatalogImage *original = &fixture->observed.image;
    SourceImageChange *changes = source_arena_allocate(&fixture->arena, original->count * sizeof(*changes)); assert(changes);
    size_t count = 0;
    for (size_t i = 0; i < original->count; i++) {
        const SourceCatalogImageEntry *old = &original->entries[i];
        if (!source_image_below(old->path, fixture->prefix)) { continue; }
        assert(old->metadata_captured && (S_ISDIR(old->metadata.st_mode) ? old->children_complete : old->content != NULL));
        SourceCatalogImageEntry row = *old; row.path = domain_mapped_path(fixture, old->path);
        row.metadata.st_dev = 0; row.metadata.st_ino = (ino_t)(i + 1000);
        SourceImageChangeKind kind = SOURCE_IMAGE_ADD;
        for (size_t j = 0; j < original->count; j++) { if (!strcmp(row.path, original->entries[j].path)) { kind = SOURCE_IMAGE_UPDATE; } }
        changes[count++] = (SourceImageChange){kind, row};
    }
    assert(count); SourceProjectionIssue issue;
    assert(source_image_project(&fixture->arena, original, changes, count, &fixture->final_root, 1, &fixture->image, &issue) == SOURCE_PROJECTION_OK);
    assert(source_image_capture_build(&fixture->final, &fixture->image) == SOURCE_IMAGE_CAPTURE_OK);
    assert(fixture->final.view.count == fixture->original.view.count && fixture->final.view.count == 4);
    for (const SourceCapturedUnit *unit = fixture->original.units; unit; unit = unit->next) {
        const SourceCloneMapping *row = NULL;
        assert(source_clone_map_add(&fixture->map, unit->source, domain_mapped_path(fixture, source_path(unit->source->text)),
            unit->source->logical_path, &row) == SOURCE_CLONE_OK);
    }
    assert(source_clone_map_seal(&fixture->map) == SOURCE_CLONE_OK);
    const DeliverySourceDefinition *definitions = fixture->analysis.original.response.definitions;
    for (const SourceCloneMapping *row = fixture->map.first; row; row = row->next) {
        const DeliverySourceDefinition *old = definitions;
        while (old && strcmp(old->identity, row->original->logical_path)) { old = old->next; }
        assert(old && fixture->edit_count < 4); size_t i = fixture->edit_count++;
        const SourceText *origin = old->dependencies ? old->dependencies->origin.source : row->original->text;
        source_text_edits_init(&fixture->edits[i], sources.allocator, origin);
        SourceManager *texts = &sources; const SourceText *text = NULL;
        assert(source_text_edits_apply(&fixture->edits[i], texts, row->path, &text) == SOURCE_EDITS_OK);
        fixture->prepared[i] = (SourceGraphPreparedText){.edits = &fixture->edits[i]};
        if (i) { fixture->prepared[i - 1].next = &fixture->prepared[i]; }
    }
    domain_compile(&fixture->after, &fixture->final.view, "generated/domain-after.json");
}
