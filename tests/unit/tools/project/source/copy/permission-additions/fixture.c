/** @file
 * @brief Prepare real cross-version clone/permission fixtures and retain exact occurrence edits.
 * @see [T224 contract](../../../../../../../docs/implementation/source-operations.md)
 */
#include "../../../../../../../tools/project/source/copy/selectors/additions.h"
/** Fixture ownership follows production lifetimes; two bounded authored source
 * definitions and their tests are the only clone candidates in this fixture. */
typedef struct {
    SourceArena arena;
    SourcePackageEnvironment original, final;
    SourceObservedImage observed;
    const char *target_identity, *target_physical;
    bool new_version;
    SourceRootCapture final_capture;
    SourceCloneMap map;
    SourceTextEdits edits[2];
    SourceGraphPreparedText provenance[2];
    size_t edit_count;
    const SourcePackageScope *scope;
    SourceSelectionRoots baseline;
    unsigned missing;
    bool cloned;
} PermissionAdditionFixture;
/** Write exact prepared bytes without assuming their buffer has a C terminator. */
static void permission_source_file(const char *path, const SourceText *text) {
    FILE *stream = fopen(path, "wb"); assert(stream);
    assert(fwrite(source_bytes(text), 1, source_length(text), stream) == source_length(text));
    assert(fclose(stream) == 0);
}
/** The independently admitted original graph supplies definition/occurrence authority. */
static const DeliverySourceDefinition *permission_definition(const char *name) {
    for (const DeliverySourceDefinition *definition = analysis.original.response.definitions; definition; definition = definition->next) {
        if (!strcmp(definition->identity, name)) { return definition; }
    }
    assert(false); return NULL;
}
/** Place one actual original main/test at an explicit same-area v2 target. */
static void permission_clone(PermissionAdditionFixture *fixture, const char *original, const char *suffix) {
    const ModuleSource *source = NULL; const SourceCloneMapping *row = NULL;
    assert(module_catalog_find(&capture.catalog, original, &source) == MODULE_OK);
    const char *identity = source_arena_join(&fixture->arena, fixture->target_identity, "", suffix);
    const char *path = source_arena_join(&fixture->arena, fixture->target_physical, "", suffix); assert(identity && path);
    const ModuleSource *absent = NULL; assert(module_catalog_find(&capture.catalog, identity, &absent) == MODULE_MISSING);
    assert(source_clone_map_add(&fixture->map, source, path, identity, &row) == SOURCE_CLONE_OK);
}
/** Root actual copied operands through C, retaining exact original-to-final site
 * evidence rather than comparing paths by guessed offsets afterward. */
static void permission_write_clones(PermissionAdditionFixture *fixture) {
    for (const SourceCloneMapping *row = fixture->map.first; row; row = row->next) {
        if (!strstr(row->identity, "/src/")) { permission_source_file(row->path, row->original->text); continue; }
        const DeliverySourceDefinition *definition = permission_definition(row->original->logical_path);
        assert(definition->dependencies && fixture->edit_count < 2);
        size_t index = fixture->edit_count++;
        source_text_edits_init(&fixture->edits[index], sources.allocator, definition->dependencies->origin.source);
        for (const DeliverySourceDependency *edge = definition->dependencies; edge; edge = edge->next) {
            SourceOperandIssue issue;
            assert(source_operand_rooted(&fixture->edits[index], &fixture->map, edge, &issue) == SOURCE_OPERAND_OK);
        }
        const SourceText *text = NULL;
        assert(source_text_edits_apply(&fixture->edits[index], &sources, row->path, &text) == SOURCE_EDITS_OK);
        permission_source_file(row->path, text);
    }
    for (size_t i = 0; i < fixture->edit_count; i++) {
        fixture->provenance[i] = (SourceGraphPreparedText){i + 1 < fixture->edit_count ? &fixture->provenance[i + 1] : NULL, &fixture->edits[i]};
    }
}
/** Original snapshots stay immutable while a distinct physical candidate image
 * receives clones. Missing-target profiles deliberately refuse before application. */
static void prepare_permission_additions(PermissionAdditionFixture *fixture, SourceCopyDependencyMode mode, bool covered, unsigned missing, bool reversed, bool new_version) {
    memset(fixture, 0, sizeof(*fixture)); fixture->cloned = mode == SOURCE_COPY_CLONE_DEPENDENCIES; fixture->missing = missing;
    fixture->new_version = new_version; assert(!new_version || !covered);
    setup_test(); fixtures();
    file("generated/project/packages/vendor/pkg/v1.0.0/src/main", reversed
        ? "@!\nstandard&\n//plain\nother&\n./other\nalso other&\n./other\n"
        : "@!\nother&\n./other\nalso other&\n./other\nstandard&\n//plain\n");
    if (!covered) { file("generated/project/packages/vendor/pkg/v2.0.0/package.ini", ""); }
    if (covered) { file("generated/project/packages/vendor/pkg/v2.0.0/package.ini", "[dependencies]\n/vendor/pkg/v1.0.0/src/other\n"); }
    capture_fixtures(); select_main();
    source_copy_closure_init(&copy, sources.allocator, &capture, &analysis, NULL);
    assert(source_copy_closure_build(&copy, &tree, mode) == SOURCE_COPY_CLOSURE_OK);
    source_arena_init(&fixture->arena, sources.allocator);
    source_package_environment_init(&fixture->original, sources.allocator);
    assert(source_package_environment_build(&fixture->original, &capture.catalog) == SOURCE_ENVIRONMENT_OK);
    source_observed_image_init(&fixture->observed, sources.allocator);
    assert(source_observed_image_build(&fixture->observed, &capture, (SourceImageBounds){0}, NULL, 0) == SOURCE_OBSERVATION_OK);
    if (new_version) {
        fixture->target_identity = "/target/new/v3.0.0";
        fixture->target_physical = source_arena_join(&fixture->arena, capture.roots[0].path, "/", "packages/target/new/v3.0.0"); assert(fixture->target_physical);
        directory("generated/project/packages/target"); directory("generated/project/packages/target/new");
        directory(fixture->target_physical);
        directory("generated/project/packages/target/new/v3.0.0/src"); directory("generated/project/packages/target/new/v3.0.0/tests");
        file("generated/project/packages/target/new/v3.0.0/package.ini", "");
    } else {
        const SourcePackageScope *target = environment_scope(&fixture->original, "/vendor/pkg/v2.0.0");
        fixture->target_identity = target->version->logical_path; fixture->target_physical = target->version->physical_path;
    }
    source_clone_map_init(&fixture->map, sources.allocator, &capture);
    permission_clone(fixture, main_identity, "/src/copied");
    permission_clone(fixture, "/vendor/pkg/v1.0.0/tests/main/check", "/tests/copied/check");
    directory(source_arena_join(&fixture->arena, fixture->target_physical, "/", "tests/copied"));
    if (fixture->cloned) {
        permission_clone(fixture, "/vendor/pkg/v1.0.0/src/other", "/src/helper");
        permission_clone(fixture, "/vendor/pkg/v1.0.0/tests/other/check", "/tests/helper/check");
        directory(source_arena_join(&fixture->arena, fixture->target_physical, "/", "tests/helper"));
    }
    assert(source_clone_map_seal(&fixture->map) == SOURCE_CLONE_OK); permission_write_clones(fixture);
    if (missing == 1) { assert(unlink("generated/project/packages/vendor/pkg/v1.0.0/src/other") == 0); }
    if (missing == 2) { assert(unlink("generated/standard/plain") == 0); }
    source_root_capture_init(&fixture->final_capture, sources.allocator);
    ModuleRoot roots[] = {{.path = "generated/project", .kind = MODULE_ROOT_PROJECT}, {.path = "generated/standard", .kind = MODULE_ROOT_STANDARD}};
    assert(source_root_capture_build(&fixture->final_capture, roots, 2) == SOURCE_CAPTURE_OK);
    source_package_environment_init(&fixture->final, sources.allocator);
    assert(source_package_environment_build(&fixture->final, &fixture->final_capture.catalog) == SOURCE_ENVIRONMENT_OK);
    fixture->scope = environment_scope(&fixture->final, fixture->target_identity);
    if (!new_version) {
        const SourcePackageScope *original_scope = environment_scope(&fixture->original, fixture->target_identity);
        SourceSelectionRootsIssue issue;
        assert(source_copy_selected_roots(&fixture->arena, original_scope->selection, fixture->final.view,
            NULL, false, NULL, 0, &fixture->baseline, &issue) == SOURCE_SELECTION_ROOTS_OK);
    }

}
/** Restore deliberate refusal inputs and retire every borrower before original
 * snapshots; remove fixture-only clone material before subsequent test profiles. */
static void finish_permission_additions(PermissionAdditionFixture *fixture) {
    if (fixture->missing) {
        const ModuleSource *source = NULL;
        const char *name = fixture->missing == 1 ? "/vendor/pkg/v1.0.0/src/other" : "//plain";
        assert(module_catalog_find(&capture.catalog, name, &source) == MODULE_OK); permission_source_file(source_path(source->text), source->text);
    }
    for (size_t i = 0; i < fixture->edit_count; i++) { source_text_edits_destroy(&fixture->edits[i]); }
    source_observed_image_destroy(&fixture->observed);
    source_package_environment_destroy(&fixture->final); source_package_environment_destroy(&fixture->original);
    source_root_capture_destroy(&fixture->final_capture);
    for (const SourceCloneMapping *row = fixture->map.first; row; row = row->next) { assert(unlink(row->path) == 0); }
    assert(rmdir(source_arena_join(&fixture->arena, fixture->target_physical, "/", "tests/copied")) == 0);
    if (fixture->cloned) { assert(rmdir(source_arena_join(&fixture->arena, fixture->target_physical, "/", "tests/helper")) == 0); }
    if (fixture->new_version) {
        assert(unlink("generated/project/packages/target/new/v3.0.0/package.ini") == 0);
        assert(rmdir("generated/project/packages/target/new/v3.0.0/src") == 0);
        assert(rmdir("generated/project/packages/target/new/v3.0.0/tests") == 0);
        assert(rmdir("generated/project/packages/target/new/v3.0.0") == 0);
        assert(rmdir("generated/project/packages/target/new") == 0);
        assert(rmdir("generated/project/packages/target") == 0);
    }
    source_clone_map_destroy(&fixture->map); source_arena_destroy(&fixture->arena); finish_copy();
}
