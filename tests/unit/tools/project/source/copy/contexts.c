/** @file
 * @brief Prove context omission through actual retained-parent/detached final images and compiler graphs.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
#include "../../../../../../tools/project/source/copy/contexts/omissions.h"
/** Select an actual child whose draft parent and flat companion are context only. */
static void select_child_context(void) {
    source_tree_selection_init(&tree, sources.allocator);
    const ModuleSource *child = NULL;
    assert(module_catalog_find(&capture.catalog, child_identity, &child) == MODULE_OK);
    assert(source_tree_selection_read(&tree, &capture, source_path(child->text)) == SOURCE_TREE_OK);
    source_copy_closure_init(&copy, sources.allocator, &capture, &analysis, NULL);
    assert(source_copy_closure_build(&copy, &tree, SOURCE_COPY_KEEP_EXTERNAL) == SOURCE_COPY_CLOSURE_OK);
    assert(copy_role("/vendor/pkg/v1.0.0/src/parent")->role == ACQUISITION_CONTEXT);
}
/** Construct the usual physical original before selecting its child. */
static void prepare_child_context(void) { prepare(); select_child_context(); }
/** Map only the selected child and its test, never its old structural parent. */
static void child_context_mains(SourceArena *arena, SourceCloneMap *mains, bool detached) {
    source_clone_map_init(mains, sources.allocator, &capture);
    const char *const originals[] = {child_identity, "/vendor/pkg/v1.0.0/tests/parent/child/check"};
    const char *const suffixes[] = {detached ? "/src/copied" : "/src/other+/copied",
        detached ? "/tests/copied/check" : "/tests/other/copied/check"};
    const char *const identities[] = {detached ? "/vendor/pkg/v1.0.0/src/copied" : "/vendor/pkg/v1.0.0/src/other/copied",
        detached ? "/vendor/pkg/v1.0.0/tests/copied/check" : "/vendor/pkg/v1.0.0/tests/other/copied/check"};
    for (size_t i = 0; i < 2; i++) {
        const ModuleSource *source = NULL; const SourceCloneMapping *row = NULL;
        assert(module_catalog_find(&capture.catalog, originals[i], &source) == MODULE_OK);
        const char *path = source_arena_join(arena, source->version->physical_path, "", suffixes[i]); assert(path);
        assert(source_clone_map_add(mains, source, path, identities[i], &row) == SOURCE_CLONE_OK);
    }
    assert(source_clone_map_seal(mains) == SOURCE_CLONE_OK);
}
/** Compose original observations with complete candidate maps and independent
 * final discovery. Existing parents are preserved, never replaced implicitly.
 */
static void it_proves_existing_parent_and_detached_context_omissions_against_real_final_images(void) {
    for (unsigned profile = 0; profile < 7; profile++) {
        prepare_child_context(); SourceArena arena; source_arena_init(&arena, sources.allocator);
        SourceCloneMap mains; child_context_mains(&arena, &mains, profile == 1);
        SourceCopyContextOmissions omissions; source_copy_context_omissions_init(&omissions, sources.allocator, &copy, &mains);
        SourceFileMap files; source_file_map_init(&files, sources.allocator); SourceCopyUnitsIssue units;
        assert(source_copy_map_unit_members(&files, &copy, &mains, &omissions, &units) == SOURCE_COPY_UNITS_OK);
        assert(omissions.first && !omissions.first->next);
        assert(source_copy_context_omissions_seal(&omissions) == SOURCE_COPY_CONTEXT_OK);
        const ModuleSource *child = tree.units[0]->source;
        SourceVersionPlacement version = {child->version->logical_path, child->version->physical_path};
        SourceCopyTestsIssue tests;
        assert(source_copy_map_test_members(&files, &copy, &mains, &version, 1, &tests) == SOURCE_COPY_TESTS_OK);
        assert(source_file_map_seal(&files) == SOURCE_FILE_MAP_OK);
        size_t mapped = 0;
        for (const SourceFileMapping *row = files.first; row; row = row->next) {
            mapped++; assert(!strstr(row->destination, "parent.extra"));
        }
        assert(mapped == 3);
        SourceObservedImage observed; source_observed_image_init(&observed, sources.allocator);
        assert(source_observed_image_build(&observed, &capture, (SourceImageBounds){0}, NULL, 0) == SOURCE_OBSERVATION_OK);
        SourceImageChange changes[6] = {0}; size_t count = 0;
        SourceManager texts; source_manager_init(&texts, sources.allocator);
        if (profile != 1) {
            const char *path = source_arena_join(&arena, version.physical, "/", "src/other+"); assert(path);
            changes[count] = (SourceImageChange){SOURCE_IMAGE_ADD, {.path = path,
                .metadata = fresh_class(&observed.image, changes, count, true), .metadata_captured = true, .children_complete = true}}; count++;
        }
        for (const SourceFileMapping *row = files.first; row; row = row->next) {
            if (profile == 6 && !strcmp(row->source, source_path(child->text))) { continue; }
            const SourceText *text = NULL;
            if (!row->original->directory) {
                assert(source_manager_add(&texts, row->destination, source_bytes(row->original->file.content),
                    source_length(row->original->file.content), &text) == SOURCE_OK);
            }
            changes[count] = (SourceImageChange){SOURCE_IMAGE_ADD, {.path = row->destination, .content = text,
                .metadata = fresh_class(&observed.image, changes, count, row->original->directory),
                .metadata_captured = true, .children_complete = row->original->directory}}; count++;
        }
        const ModuleSource *parent = NULL;
        assert(module_catalog_find(&capture.catalog, "/vendor/pkg/v1.0.0/src/other", &parent) == MODULE_OK);
        if (profile == 2) { changes[count++] = (SourceImageChange){SOURCE_IMAGE_REMOVE, {.path = source_path(parent->text)}}; }
        if (profile == 3) {
            const SourceCatalogImageEntry *entry = observed.image.entries;
            while (strcmp(entry->path, source_path(parent->text))) { entry++; }
            const SourceText *text = NULL;
            assert(source_manager_add(&texts, entry->path, (const unsigned char *)"@!\n", 3, &text) == SOURCE_OK);
            changes[count] = (SourceImageChange){SOURCE_IMAGE_UPDATE, *entry}; changes[count++].entry.content = text;
        }
        if (profile == 4) {
            const char *path = source_arena_join(&arena, source_path(parent->text), "", ".extra"); assert(path);
            const SourceText *text = NULL;
            assert(source_manager_add(&texts, path, (const unsigned char *)"new companion", 13, &text) == SOURCE_OK);
            changes[count] = (SourceImageChange){SOURCE_IMAGE_ADD, {.path = path, .content = text,
                .metadata = fresh_class(&observed.image, changes, count, false), .metadata_captured = true}}; count++;
        }
        SourceCatalogImage image; SourceProjectionIssue projection;
        assert(source_image_project(&arena, &observed.image, changes, count, (&observed.image)->roots, (&observed.image)->root_count, &image, &projection) == SOURCE_PROJECTION_OK);
        SourceImageCapture final; source_image_capture_init(&final, sources.allocator);
        SourceImageCaptureStatus captured = source_image_capture_build(&final, &image);
        SourceCopyContextIssue issue;
        const char *removed[] = {parent->logical_path}; size_t before = allocation_count;
        if (profile == 2) {
            assert(captured != SOURCE_IMAGE_CAPTURE_OK && !final.valid);
            assert(source_copy_context_omissions_check(&omissions, &observed, &final, NULL, 0, &issue) == SOURCE_COPY_CONTEXT_INVALID);
        } else {
            assert(captured == SOURCE_IMAGE_CAPTURE_OK);
            SourceCopyContextStatus expected = profile < 2 ? SOURCE_COPY_CONTEXT_OK
                : profile < 5 ? SOURCE_COPY_CONTEXT_CHANGED : SOURCE_COPY_CONTEXT_MISSING;
            assert(source_copy_context_omissions_check(&omissions, &observed, &final, removed, profile == 5 ? 1 : 0, &issue) == expected);
            assert(allocation_count == before);
            if (profile < 2) {
                assert(!issue.context && !issue.requester && !issue.parent);
                FILE *request = fopen("generated/context-final.json", "wb"); assert(request);
                assert(delivery_source_snapshot_write(request, &final.view) && fclose(request) == 0);
                SourceCompilerClient compiler; source_compiler_client_init(&compiler, sources.allocator);
                assert(source_compiler_client_run(&compiler, "../../../../../../ama", "generated/context-final.json", &final.view, NULL) == SOURCE_COMPILER_OK);
                SourceSitesIssue sites;
                assert(source_graph_sites(analysis.original.response.definitions, compiler.response.definitions, &mains, NULL, 0, (SourceRegistryRetirement){0}, NULL, &sites) == SOURCE_SITES_OK);
                source_compiler_client_destroy(&compiler);
                assert(source_copy_context_omissions_check(&omissions, &observed, &final, NULL, 0, &issue) == SOURCE_COPY_CONTEXT_OK);
            } else { assert(issue.context && issue.requester); }
        }
        assert(source_copy_context_omissions_check(&omissions, &observed, &final, NULL, 1, &issue) == SOURCE_COPY_CONTEXT_INVALID);
        assert(source_copy_context_omissions_check(&omissions, &observed, &final, NULL, 0, NULL) == SOURCE_COPY_CONTEXT_INVALID);
        source_image_capture_destroy(&final); source_manager_destroy(&texts); source_observed_image_destroy(&observed);
        source_file_map_destroy(&files); source_copy_context_omissions_destroy(&omissions); source_clone_map_destroy(&mains);
        source_arena_destroy(&arena); finish_copy();
    }
}
/** Context omission is explicit, complete and allocation-safe; active and foreign
 * roles cannot be omitted even when the caller supplies an obligation owner.
 */
static void it_accounts_for_context_omissions_without_losing_active_units_or_failed_prefixes(void) {
    prepare_child_context(); SourceArena arena; source_arena_init(&arena, sources.allocator);
    SourceCloneMap mains; child_context_mains(&arena, &mains, false);
    const AcquisitionRoleRecord *context = copy_role("/vendor/pkg/v1.0.0/src/parent");
    const AcquisitionRoleRecord *active = copy_role(child_identity);
    SourceCopyContextOmissions owner;
    source_copy_context_omissions_init(&owner, sources.allocator, &copy, &mains);
    assert(source_copy_context_omissions_seal(&owner) == SOURCE_COPY_CONTEXT_MISSING);
    source_copy_context_omissions_destroy(&owner);
    for (unsigned invalid = 0; invalid < 3; invalid++) {
        source_copy_context_omissions_init(&owner, sources.allocator, &copy, &mains);
        AcquisitionRoleRecord foreign = *context;
        assert(source_copy_context_omit(&owner, invalid == 0 ? active : invalid == 1 ? &foreign : NULL) == SOURCE_COPY_CONTEXT_INVALID);
        assert(source_copy_context_omissions_seal(&owner) == SOURCE_COPY_CONTEXT_INVALID);
        source_copy_context_omissions_destroy(&owner);
    }
    size_t total = 0, baseline = live_allocations;
    for (size_t failure = 0; !failure || failure <= total; failure++) {
        source_copy_context_omissions_init(&owner, sources.allocator, &copy, &mains);
        SourceFileMap files; source_file_map_init(&files, sources.allocator); SourceCopyUnitsIssue issue;
        size_t before = allocation_count; if (failure) { failure_at = before + failure; }
        SourceCopyUnitsStatus status = source_copy_map_unit_members(&files, &copy, &mains, &owner, &issue);
        if (!failure) {
            assert(status == SOURCE_COPY_UNITS_OK); total = allocation_count - before; assert(total);
            size_t repeat = allocation_count;
            assert(source_copy_context_omit(&owner, context) == SOURCE_COPY_CONTEXT_OK && allocation_count == repeat);
            assert(source_copy_context_omissions_seal(&owner) == SOURCE_COPY_CONTEXT_OK);
            assert(source_copy_context_omit(&owner, context) == SOURCE_COPY_CONTEXT_INVALID && owner.ready);
            assert(source_copy_context_omissions_seal(&owner) == SOURCE_COPY_CONTEXT_INVALID);
            assert(source_copy_map_unit_members(&files, &copy, &mains, &owner, &issue) == SOURCE_COPY_UNITS_CONTEXT);
        } else { assert(status == SOURCE_COPY_UNITS_MEMORY && source_file_map_seal(&files) != SOURCE_FILE_MAP_OK); }
        failure_at = 0; source_file_map_destroy(&files); source_copy_context_omissions_destroy(&owner);
        assert(live_allocations == baseline);
    }
    SourceCloneMap empty; source_clone_map_init(&empty, sources.allocator, &capture);
    assert(source_clone_map_seal(&empty) == SOURCE_CLONE_OK);
    source_copy_context_omissions_init(&owner, sources.allocator, &copy, &empty);
    SourceFileMap files; source_file_map_init(&files, sources.allocator); SourceCopyUnitsIssue issue;
    assert(source_copy_map_unit_members(&files, &copy, &empty, &owner, &issue) == SOURCE_COPY_UNITS_MAIN);
    assert(source_copy_context_omissions_seal(&owner) == SOURCE_COPY_CONTEXT_MISSING);
    source_file_map_destroy(&files); source_copy_context_omissions_destroy(&owner); source_clone_map_destroy(&empty);
    source_clone_map_destroy(&mains); source_arena_destroy(&arena); finish_copy();
}
