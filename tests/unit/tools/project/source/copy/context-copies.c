/** @file
 * @brief Check actual required, existing and detached mapped context parents through final compiler proofs.
 * @see [T276 resumed review](../../../../../../docs/implementation/source-operations.md#t276-resumed-entry-review-after-t285)
 */
/** Bind genuine source, parent and test handles to one explicit candidate layout.
 * Profile one deliberately proposes replacing the same existing draft parent;
 * profile two proposes an unrelated new parent which no copied child requires. */
static void context_copy_mains(SourceArena *arena, SourceCloneMap *mains, unsigned profile) {
    source_clone_map_init(mains, sources.allocator, &capture);
    const char *originals[] = {"/vendor/pkg/v1.0.0/src/parent", child_identity, "/vendor/pkg/v1.0.0/tests/parent/child/check"};
    const char *suffixes[] = {profile == 1 ? "/src/parent" : profile == 2 ? "/src/spare" : "/src/needed",
        profile == 1 ? "/src/parent+/copied" : profile == 2 ? "/src/copied" : "/src/needed+/copied",
        profile == 1 ? "/tests/parent/copied/check" : profile == 2 ? "/tests/copied/check" : "/tests/needed/copied/check"};
    const char *identities[] = {profile == 1 ? "/vendor/pkg/v1.0.0/src/parent" : profile == 2 ? "/vendor/pkg/v1.0.0/src/spare" : "/vendor/pkg/v1.0.0/src/needed",
        profile == 1 ? "/vendor/pkg/v1.0.0/src/parent/copied" : profile == 2 ? "/vendor/pkg/v1.0.0/src/copied" : "/vendor/pkg/v1.0.0/src/needed/copied",
        profile == 1 ? "/vendor/pkg/v1.0.0/tests/parent/copied/check" : profile == 2 ? "/vendor/pkg/v1.0.0/tests/copied/check" : "/vendor/pkg/v1.0.0/tests/needed/copied/check"};
    for (size_t i = 0; i < 3; i++) {
        const ModuleSource *source = NULL; const SourceCloneMapping *row = NULL;
        assert(module_catalog_find(&capture.catalog, originals[i], &source) == MODULE_OK);
        const char *path = source_arena_join(arena, source->version->physical_path, "", suffixes[i]); assert(path);
        assert(source_clone_map_add(mains, source, path, identities[i], &row) == SOURCE_CLONE_OK);
    }
    assert(source_clone_map_seal(mains) == SOURCE_CLONE_OK);
}
/** All three final images really compile. The missing context-necessity gate
 * would otherwise accept an unnecessary same-byte parent replacement or orphan
 * context copy which neither structural correctness nor graph equality detects. */
static void it_proves_context_copy_necessity_and_refuses_existing_or_detached_parent_clones(void) {
    for (unsigned profile = 0; profile < 3; profile++) {
        prepare_child_context(); SourceArena arena; source_arena_init(&arena, sources.allocator);
        SourceCloneMap mains; context_copy_mains(&arena, &mains, profile);
        SourceCopyContextOmissions omissions; source_copy_context_omissions_init(&omissions, sources.allocator, &copy, &mains);
        SourceFileMap files; source_file_map_init(&files, sources.allocator); SourceCopyUnitsIssue units;
        assert(source_copy_map_unit_members(&files, &copy, &mains, &omissions, &units) == SOURCE_COPY_UNITS_OK);
        assert(source_copy_context_omissions_seal(&omissions) == SOURCE_COPY_CONTEXT_OK && !omissions.first);
        const ModuleSource *child = tree.units[0]->source;
        SourceVersionPlacement version = {child->version->logical_path, child->version->physical_path}; SourceCopyTestsIssue tests;
        assert(source_copy_map_test_members(&files, &copy, &mains, &version, 1, &tests) == SOURCE_COPY_TESTS_OK);
        assert(source_file_map_seal(&files) == SOURCE_FILE_MAP_OK);
        SourceObservedImage observed; source_observed_image_init(&observed, sources.allocator);
        assert(source_observed_image_build(&observed, &capture, (SourceImageBounds){0}, NULL, 0) == SOURCE_OBSERVATION_OK);
        SourceImageChange changes[2]; size_t count = 0;
        if (profile == 1) {
            for (const SourceFileMapping *row = files.first; row; row = row->next) {
                if (strcmp(row->source, row->destination)) { continue; }
                for (size_t i = 0; i < observed.image.count; i++) {
                    if (!strcmp(observed.image.entries[i].path, row->source)) {
                        assert(count < 2); changes[count++] = (SourceImageChange){SOURCE_IMAGE_REMOVE, observed.image.entries[i]};
                    }
                }
            }
            assert(count == 2);
        }
        const SourceCatalogImage *image = NULL; SourceCopyImageIssue image_issue;
        assert(source_copy_image_prepare(&arena, &observed.image, &files, NULL, changes, count, observed.image.roots,
            observed.image.root_count, &image, &image_issue) == SOURCE_COPY_IMAGE_OK);
        SourceImageCapture final; source_image_capture_init(&final, sources.allocator);
        assert(source_image_capture_build(&final, image) == SOURCE_IMAGE_CAPTURE_OK);
        FILE *request = fopen("generated/context-copy.json", "wb"); assert(request);
        assert(delivery_source_snapshot_write(request, &final.view) && fclose(request) == 0);
        SourceCompilerClient compiler; source_compiler_client_init(&compiler, sources.allocator);
        assert(source_compiler_client_run(&compiler, "../../../../../../ama", "generated/context-copy.json", &final.view, NULL) == SOURCE_COMPILER_OK);
        SourceCopyContextIssue context_issue; size_t before = allocation_count;
        assert(source_copy_context_copies_check(&omissions, &observed, &final, &context_issue)
            == (profile ? SOURCE_COPY_CONTEXT_UNNECESSARY : SOURCE_COPY_CONTEXT_OK));
        assert(allocation_count == before);
        SourceCopyProofIssue proof;
        assert(source_copy_prove(&arena, &analysis, &observed, &mains, &files, NULL, &omissions,
            &final, &compiler, NULL, 0, &proof) == (profile ? SOURCE_COPY_PROOF_CONTEXT : SOURCE_COPY_PROOF_OK));
        if (profile == 1) {
            /* Explicitly selecting the original parent changes its authority:
             * this one gate must defer its copy to normal raw/collision rules. */
            SourceTreeSelection explicit_tree; source_tree_selection_init(&explicit_tree, sources.allocator);
            const ModuleSource *parent = NULL;
            assert(module_catalog_find(&capture.catalog, "/vendor/pkg/v1.0.0/src/parent", &parent) == MODULE_OK);
            assert(source_tree_selection_read(&explicit_tree, &capture, source_path(parent->text)) == SOURCE_TREE_OK);
            SourceCopyClosure explicit_copy; source_copy_closure_init(&explicit_copy, sources.allocator, &capture, &analysis, NULL);
            assert(source_copy_closure_build(&explicit_copy, &explicit_tree, SOURCE_COPY_KEEP_EXTERNAL) == SOURCE_COPY_CLOSURE_OK);
            SourceCopyContextOmissions explicit_context;
            source_copy_context_omissions_init(&explicit_context, sources.allocator, &explicit_copy, &mains);
            assert(source_copy_context_omissions_seal(&explicit_context) == SOURCE_COPY_CONTEXT_OK);
            assert(source_copy_context_copies_check(&explicit_context, &observed, &final, &context_issue) == SOURCE_COPY_CONTEXT_OK);
            source_copy_context_omissions_destroy(&explicit_context); source_copy_closure_destroy(&explicit_copy);
            source_tree_selection_destroy(&explicit_tree);
        }
        assert(source_copy_context_copies_check(NULL, &observed, &final, &context_issue) == SOURCE_COPY_CONTEXT_INVALID);
        assert(source_copy_context_copies_check(&omissions, NULL, &final, &context_issue) == SOURCE_COPY_CONTEXT_INVALID);
        assert(source_copy_context_copies_check(&omissions, &observed, &final, NULL) == SOURCE_COPY_CONTEXT_INVALID);
        source_compiler_client_destroy(&compiler); source_image_capture_destroy(&final); source_observed_image_destroy(&observed);
        source_file_map_destroy(&files); source_copy_context_omissions_destroy(&omissions); source_clone_map_destroy(&mains);
        source_arena_destroy(&arena); finish_copy();
    }
}
