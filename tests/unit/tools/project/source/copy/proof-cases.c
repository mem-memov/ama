/** @file
 * @brief Independently compile incorrect final images and reject their physical copy laws.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
/** Valid compiler inputs can still violate copy preservation: whitespace edits to
 * originals, leftover copied companions and changed opaque subtree bytes do not
 * require invalid Ama syntax or a graph mismatch. Prove the separate refusals. */
static void check_copy_proof_refusals(SourceArena *arena, SourceManager *texts,
    const SourceCloneMap *map, const SourceFileMap *files, const SourceGraphPreparedText *prepared,
    const SourceCopyContextOmissions *contexts, const SourceImageCapture *accepted, const SourceCompilerClient *accepted_client,
    const char *opaque_path) {
    SourceCopyProofIssue proof;
    assert(source_copy_prove(arena, &analysis, map, files, prepared, NULL, accepted, accepted_client, NULL, 0, &proof) == SOURCE_COPY_PROOF_INVALID);
    const char *invalid[] = {NULL};
    assert(source_copy_prove(arena, &analysis, map, files, prepared, contexts, accepted, accepted_client, invalid, 1, &proof) == SOURCE_COPY_PROOF_INVALID);
    for (unsigned variant = 0; variant < 3; variant++) {
        const SourceCatalogImage *image = accepted->image;
        SourceImageChange change = {.kind = variant == 1 ? SOURCE_IMAGE_ADD : SOURCE_IMAGE_UPDATE};
        const char *path = variant == 0 ? source_path(tree.units[0]->source->text) : variant == 2 ? opaque_path : NULL;
        if (variant == 1) {
            path = source_arena_join(arena, map->first->path, ".", "unexpected"); assert(path);
            change.entry = (SourceCatalogImageEntry){.path = path, .metadata = fresh_class(image, NULL, 0, false), .metadata_captured = true};
        } else {
            for (size_t i = 0; i < image->count; i++) { if (!strcmp(image->entries[i].path, path)) { change.entry = image->entries[i]; break; } }
            assert(change.entry.content);
        }
        const unsigned char *bytes = (const unsigned char *)"changed opaque material"; size_t length = strlen((const char *)bytes);
        if (variant == 0) {
            size_t old_length = source_length(change.entry.content);
            unsigned char *extended = source_arena_allocate(arena, old_length + 1); assert(extended);
            memcpy(extended, source_bytes(change.entry.content), old_length); extended[old_length] = '\n';
            bytes = extended; length = old_length + 1;
        }
        assert(source_manager_add(texts, path, bytes, length, &change.entry.content) == SOURCE_OK);
        SourceCatalogImage changed; SourceProjectionIssue projection;
        assert(source_image_project(arena, image, &change, 1, (image)->roots, (image)->root_count, &changed, &projection) == SOURCE_PROJECTION_OK);
        SourceImageCapture captured; source_image_capture_init(&captured, sources.allocator);
        assert(source_image_capture_build(&captured, &changed) == SOURCE_IMAGE_CAPTURE_OK);
        FILE *request = fopen("generated/final-copy-refusal.json", "wb"); assert(request);
        assert(delivery_source_snapshot_write(request, &captured.view) && fclose(request) == 0);
        SourceCompilerClient client; source_compiler_client_init(&client, sources.allocator);
        assert(source_compiler_client_run(&client, "../../../../../../ama", "generated/final-copy-refusal.json", &captured.view, NULL) == SOURCE_COMPILER_OK);
        SourceCopyProofStatus status = source_copy_prove(arena, &analysis, map, files, prepared, contexts, &captured, &client, NULL, 0, &proof);
        assert(status == (variant == 0 ? SOURCE_COPY_PROOF_ORIGINAL : variant == 1 ? SOURCE_COPY_PROOF_PLACEMENT : SOURCE_COPY_PROOF_MATERIAL));
        assert(proof.path && !strcmp(proof.path, path));
        source_compiler_client_destroy(&client); source_image_capture_destroy(&captured);
    }
}
