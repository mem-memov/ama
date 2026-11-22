/** @file
 * @brief Exercise explicit input placement, version-relative assets and shared-cause conflicts.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
#include "../../../../../../tools/project/source/copy/members/declared.h"
/** Use the actual selected module handle as the input's requiring owner. */
static const SourceCapturedUnit *declared_owner(const char *identity) {
    for (const SourceCapturedUnit *unit = capture.units; unit; unit = unit->next) {
        if (!strcmp(unit->source->logical_path, identity)) { return unit; }
    }
    assert(false); return NULL;
}
/** Whole version identities and absolute roots are independent of root spelling
 * inside ordinary names; invalid unused descriptors cannot hide behind a match. */
static void it_validates_complete_explicit_version_placement_tables_without_allocation(void) {
    assert(source_version_placements_valid(NULL, 0));
    assert(!source_version_placements_valid(NULL, 1));
    for (unsigned profile = 0; profile < 9; profile++) {
        SourceVersionPlacement versions[] = {{"/v/p/v1.0.0", "/a"}, {"/v/p/v2.0.0", "/b/"}};
        if (profile == 1) { versions[1].identity = NULL; }
        if (profile == 2) { versions[1].physical = NULL; }
        if (profile == 3) { versions[1].physical = "relative"; }
        if (profile == 4) { versions[1].identity = "//standard"; }
        if (profile == 5) { versions[1].identity = "/v/p/v2.0.0/src/main"; }
        if (profile == 6) { versions[1].identity = versions[0].identity; }
        if (profile == 7) { versions[1].identity = "/v/p"; }
        if (profile == 8) { versions[1].physical = ""; }
        assert(source_version_placements_valid(versions, 2) == (profile == 0));
    }
}
/** Capture a declared opaque file under a known original version root. */
static const char *declared_file(SourceArena *arena, SourceManager *files, AcquisitionUnitEntry *entry) {
    const char *root = declared_owner(main_identity)->source->version->physical_path;
    const char *path = source_arena_join(arena, root, "/", "declared.data"); assert(path);
    file(path, "\xff explicit input\n");
    *entry = (AcquisitionUnitEntry){0};
    assert(delivery_file_capture(arena, files, root, "declared.data", &entry->file) == SOURCE_OK);
    assert(entry->file.state == DELIVERY_FILE_PRESENT);
    return path;
}
/** Retire declaration borrowers before their bytes, then the original owner and
 * counted allocator. Preserve the owned path until its fixture file is removed.
 */
static void finish_declared_material(const char *path, SourceManager *texts, SourceArena *arena) {
    source_copy_closure_destroy(&copy);
    source_tree_selection_destroy(&tree);
    assert(unlink(path) == 0);
    source_manager_destroy(texts);
    source_arena_destroy(arena);
    finish();
}
/** Bound assets retain version-relative suffixes, while existing structural maps
 * take priority and explicitly unversioned assets require a supplied placement. */
static void it_maps_declared_assets_with_explicit_origins_and_preserves_environment_ownership(void) {
    for (unsigned profile = 0; profile < 6; profile++) {
        prepare(); select_main(); SourceArena arena; source_arena_init(&arena, sources.allocator);
        SourceManager texts; source_manager_init(&texts, sources.allocator);
        AcquisitionUnitEntry asset; const char *path = declared_file(&arena, &texts, &asset);
        const SourceCapturedUnit *main = declared_owner(main_identity);
        AcquisitionUnitEntry configuration = {.file = {.path = source_path(main->source->version->configuration),
            .content = main->source->version->configuration, .state = DELIVERY_FILE_PRESENT, .issue = PATH_FOUND}};
        asset.next = &configuration;
        SourceDeclaredGroup group = {.requester = main, .version = profile == 2 || profile == 3 ? NULL : main->source->version, .files = &asset};
        source_copy_closure_init(&copy, sources.allocator, &capture, &analysis, &group);
        assert(source_copy_closure_build(&copy, &tree, SOURCE_COPY_CLONE_DEPENDENCIES) == SOURCE_COPY_CLOSURE_OK);
        SourceCloneMap explicit_map; main_destination(&explicit_map);
        SourceCopyDestinations destinations; source_copy_destinations_init(&destinations, sources.allocator);
        assert(source_copy_destinations_build(&destinations, &copy, &explicit_map) == SOURCE_COPY_DESTINATIONS_OK);
        SourceVersionPlacement version = {"/target/new/v3.0.0", "/prepared/"};
        SourceFileMap files; source_file_map_init(&files, sources.allocator);
        if (profile == 1 || profile == 3 || profile == 5) {
            const SourceFileMapping *row = NULL;
            const SourceText *content = asset.file.content;
            if (profile == 5) { assert(source_manager_add(&texts, "different", (const unsigned char *)"changed", 7, &content) == SOURCE_OK); }
            assert(source_file_map_observation(&files, path, content, false, "/explicit/input.data", &row) == SOURCE_FILE_MAP_OK);
        }
        SourceCopyDeclaredIssue issue;
        SourceCopyDeclaredStatus status = source_copy_map_declared_members(&files, &destinations, &version, profile == 4 ? 0 : 1, &issue);
        SourceCopyDeclaredStatus expected = profile == 2 ? SOURCE_COPY_DECLARED_UNPLACED : profile == 4 ? SOURCE_COPY_DECLARED_VERSION
            : profile == 5 ? SOURCE_COPY_DECLARED_MAP : SOURCE_COPY_DECLARED_OK;
        assert(status == expected);
        if (expected == SOURCE_COPY_DECLARED_OK) {
            assert(files.first && !files.first->next && !strcmp(files.first->source, path));
            assert(!strcmp(files.first->destination, profile == 0 ? "/prepared/declared.data" : "/explicit/input.data"));
            assert(source_file_map_seal(&files) == SOURCE_FILE_MAP_OK);
        } else { assert(source_file_map_seal(&files) != SOURCE_FILE_MAP_OK && !files.ready); }
        source_file_map_destroy(&files); source_copy_destinations_destroy(&destinations); source_clone_map_destroy(&explicit_map);
        finish_declared_material(path, &texts, &arena);
    }
}
/** Two actual source requesters assigned different destination versions cannot
 * silently copy their shared asset to the first requester's chosen location. */
static void it_refuses_conflicting_shared_declared_input_destinations(void) {
    prepare(); SourceArena arena; source_arena_init(&arena, sources.allocator);
    SourceManager texts; source_manager_init(&texts, sources.allocator);
    AcquisitionUnitEntry asset; const char *path = declared_file(&arena, &texts, &asset);
    const SourceCapturedUnit *main = declared_owner(main_identity), *other = declared_owner("/vendor/pkg/v1.0.0/src/other");
    SourceDeclaredGroup back = {.requester = other, .version = other->source->version, .files = &asset};
    SourceDeclaredGroup forward = {.next = &back, .requester = main, .version = main->source->version, .files = &asset};
    source_tree_selection_init(&tree, sources.allocator);
    assert(source_tree_selection_read(&tree, &capture, "generated/project/packages/vendor/pkg/v1.0.0/src") == SOURCE_TREE_OK);
    source_copy_closure_init(&copy, sources.allocator, &capture, &analysis, &forward);
    assert(source_copy_closure_build(&copy, &tree, SOURCE_COPY_CLONE_DEPENDENCIES) == SOURCE_COPY_CLOSURE_OK);
    SourceCloneMap explicit_map; source_clone_map_init(&explicit_map, sources.allocator, &capture);
    for (size_t i = 0; i < tree.count; i++) {
        const ModuleSource *source = tree.units[i]->source;
        const char *suffix = source->logical_path + strlen("/vendor/pkg/v1.0.0/src/");
        const char *identity = source_arena_join(&arena, source == other->source ? "/target/b/v2.0.0/src/" : "/target/a/v2.0.0/src/", "", suffix);
        const char *destination = source_arena_join(&arena, "/prepared/src/", "", suffix);
        const SourceCloneMapping *row = NULL; assert(identity && destination);
        assert(source_clone_map_add(&explicit_map, source, destination, identity, &row) == SOURCE_CLONE_OK);
    }
    SourceCopyDestinations destinations; source_copy_destinations_init(&destinations, sources.allocator);
    assert(source_copy_destinations_build(&destinations, &copy, &explicit_map) == SOURCE_COPY_DESTINATIONS_OK);
    SourceVersionPlacement versions[] = {{"/target/a/v2.0.0", "/prepared/a"}, {"/target/b/v2.0.0", "/prepared/b"}};
    SourceFileMap files; source_file_map_init(&files, sources.allocator); SourceCopyDeclaredIssue issue;
    assert(source_copy_map_declared_members(&files, &destinations, versions, 2, &issue) == SOURCE_COPY_DECLARED_MAP);
    assert(files.status == SOURCE_FILE_MAP_CONFLICT && files.conflict && files.requester && issue.member && issue.cause);
    assert(!strcmp(files.conflict->source, path) && !strcmp(files.requester->file.path, path));
    assert(source_file_map_seal(&files) != SOURCE_FILE_MAP_OK);
    source_file_map_destroy(&files); source_copy_destinations_destroy(&destinations); source_clone_map_destroy(&explicit_map);
    finish_declared_material(path, &texts, &arena);
}
/** Every declared path/adapter/row allocation retires to the original baseline;
 * invalid handoffs cannot publish a complete result or modify a closed owner. */
static void it_retires_every_declared_material_allocation_and_refuses_invalid_interfaces(void) {
    prepare(); select_main(); SourceArena arena; source_arena_init(&arena, sources.allocator);
    SourceManager texts; source_manager_init(&texts, sources.allocator);
    AcquisitionUnitEntry asset; const char *path = declared_file(&arena, &texts, &asset);
    const SourceCapturedUnit *main = declared_owner(main_identity);
    SourceDeclaredGroup group = {.requester = main, .version = main->source->version, .files = &asset};
    source_copy_closure_init(&copy, sources.allocator, &capture, &analysis, &group);
    assert(source_copy_closure_build(&copy, &tree, SOURCE_COPY_CLONE_DEPENDENCIES) == SOURCE_COPY_CLOSURE_OK);
    SourceCloneMap explicit_map; main_destination(&explicit_map);
    SourceCopyDestinations destinations; source_copy_destinations_init(&destinations, sources.allocator);
    assert(source_copy_destinations_build(&destinations, &copy, &explicit_map) == SOURCE_COPY_DESTINATIONS_OK);
    SourceVersionPlacement version = {"/target/new/v3.0.0", "/prepared"};
    size_t total = 0, baseline = live_allocations; SourceCopyDeclaredIssue issue;
    for (size_t failure = 0; !failure || failure <= total; failure++) {
        SourceFileMap files; source_file_map_init(&files, sources.allocator);
        size_t before = allocation_count; if (failure) { failure_at = before + failure; }
        SourceCopyDeclaredStatus status = source_copy_map_declared_members(&files, &destinations, &version, 1, &issue);
        if (!failure) { assert(status == SOURCE_COPY_DECLARED_OK); total = allocation_count - before; assert(total); }
        else { assert(status == SOURCE_COPY_DECLARED_MEMORY && source_file_map_seal(&files) != SOURCE_FILE_MAP_OK); }
        failure_at = 0; source_file_map_destroy(&files); assert(live_allocations == baseline);
    }
    SourceFileMap files; source_file_map_init(&files, sources.allocator);
    assert(source_copy_map_declared_members(NULL, &destinations, &version, 1, &issue) == SOURCE_COPY_DECLARED_INVALID);
    assert(source_copy_map_declared_members(&files, &destinations, &version, 1, NULL) == SOURCE_COPY_DECLARED_INVALID && files.status == SOURCE_FILE_MAP_OK);
    assert(source_copy_map_declared_members(&files, NULL, &version, 1, &issue) == SOURCE_COPY_DECLARED_INVALID);
    source_file_map_destroy(&files); source_file_map_init(&files, sources.allocator);
    assert(source_copy_map_declared_members(&files, &destinations, NULL, 1, &issue) == SOURCE_COPY_DECLARED_VERSION);
    source_file_map_destroy(&files); source_file_map_init(&files, sources.allocator);
    assert(source_file_map_seal(&files) == SOURCE_FILE_MAP_OK);
    assert(source_copy_map_declared_members(&files, &destinations, &version, 1, &issue) == SOURCE_COPY_DECLARED_INVALID && files.ready);
    source_file_map_destroy(&files); source_copy_destinations_destroy(&destinations); source_clone_map_destroy(&explicit_map);
    finish_declared_material(path, &texts, &arena);
}
