/** @file
 * @brief Prepare real ordinary/standard module companions and complete mapped unit/test material.
 * @see [T276 resumed review](../../../../../../../docs/implementation/source-operations.md#t276-resumed-entry-review-after-t285)
 */
#include "../../../../../../../tools/project/source/operation/writes/module/module.h"
/** Own wrapper facts; the shared leaf owns the genuine admitted original scope. */
typedef struct {
    SourceArena arena;
    SourceManager texts;
    SourcePackageEnvironment original;
    SourceObservedImage observed;
    SourceCloneMap mains;
    SourceFileMap files;
    SourceCopyContextOmissions contexts;
    SourceTextEdits main_edits;
    const SourceCloneMapping *clone;
    SourceOperationModuleWriteInputs inputs;
} ModuleWriteFixture;
/** Construct aliases before original admission; unused targets import no module. */
static void module_write_fixture(ModuleWriteFixture *fixture, bool standard) {
    setup_test(); fixtures();
    const char *main = standard ? "//plain" : main_identity;
    file(standard ? "generated/standard/plain.ini" : "generated/project/packages/vendor/pkg/v1.0.0/src/main.ini",
        standard ? "; local aliases\n~local = ./missing\n" : "; local aliases\n~local = ./other\n");
    capture_fixtures(); source_tree_selection_init(&tree, sources.allocator);
    const ModuleSource *source = NULL;
    assert(module_catalog_find(&capture.catalog, main, &source) == MODULE_OK);
    assert(source_tree_selection_read(&tree, &capture, source_path(source->text)) == SOURCE_TREE_OK);
    source_copy_closure_init(&copy, sources.allocator, &capture, &analysis, NULL);
    assert(source_copy_closure_build(&copy, &tree, SOURCE_COPY_KEEP_EXTERNAL) == SOURCE_COPY_CLOSURE_OK);
    memset(fixture, 0, sizeof(*fixture)); source_arena_init(&fixture->arena, sources.allocator);
    source_manager_init(&fixture->texts, sources.allocator); source_package_environment_init(&fixture->original, sources.allocator);
    assert(source_package_environment_build(&fixture->original, &capture.catalog) == SOURCE_ENVIRONMENT_OK);
    source_observed_image_init(&fixture->observed, sources.allocator);
    assert(source_observed_image_build(&fixture->observed, &capture, (SourceImageBounds){0}, NULL, 0) == SOURCE_OBSERVATION_OK);
    source_clone_map_init(&fixture->mains, sources.allocator, &capture);
    const char *path = source_arena_join(&fixture->arena, standard ? capture.roots[1].path : source->version->physical_path,
        "/", standard ? "copied" : "src/namespace/copied"); assert(path);
    assert(source_clone_map_add(&fixture->mains, source, path,
        standard ? "//copied" : "/vendor/pkg/v1.0.0/src/namespace/copied", &fixture->clone) == SOURCE_CLONE_OK);
    if (!standard) {
        const ModuleSource *test = NULL; const SourceCloneMapping *row = NULL;
        assert(module_catalog_find(&capture.catalog, "/vendor/pkg/v1.0.0/tests/main/check", &test) == MODULE_OK);
        path = source_arena_join(&fixture->arena, source->version->physical_path, "/", "tests/namespace/copied/check"); assert(path);
        assert(source_clone_map_add(&fixture->mains, test, path, "/vendor/pkg/v1.0.0/tests/namespace/copied/check", &row) == SOURCE_CLONE_OK);
    }
    assert(source_clone_map_seal(&fixture->mains) == SOURCE_CLONE_OK);
    source_copy_context_omissions_init(&fixture->contexts, sources.allocator, &copy, &fixture->mains);
    source_file_map_init(&fixture->files, sources.allocator); SourceCopyUnitsIssue units;
    assert(source_copy_map_unit_members(&fixture->files, &copy, &fixture->mains, &fixture->contexts, &units) == SOURCE_COPY_UNITS_OK);
    assert(source_copy_context_omissions_seal(&fixture->contexts) == SOURCE_COPY_CONTEXT_OK);
    if (!standard) {
        SourceVersionPlacement version = {source->version->logical_path, source->version->physical_path}; SourceCopyTestsIssue tests;
        assert(source_copy_map_test_members(&fixture->files, &copy, &fixture->mains, &version, 1, &tests) == SOURCE_COPY_TESTS_OK);
    }
    assert(source_file_map_seal(&fixture->files) == SOURCE_FILE_MAP_OK);
    fixture->inputs = (SourceOperationModuleWriteInputs){.copy = &copy, .mains = &fixture->mains,
        .files = &fixture->files, .original = standard ? NULL : &fixture->original, .clone = fixture->clone};
    const DeliverySourceDefinition *definition = copy_definition(main);
    const SourceText *original = definition->dependencies ? definition->dependencies->origin.source : source->text;
    source_text_edits_init(&fixture->main_edits, sources.allocator, original);
    for (const DeliverySourceDependency *edge = definition->dependencies; edge; edge = edge->next) {
        SourceOperandIssue issue;
        assert(source_operand_rooted(&fixture->main_edits, &fixture->mains, edge, &issue) == SOURCE_OPERAND_OK);
    }
    const SourceText *result = NULL;
    assert(source_text_edits_apply(&fixture->main_edits, &fixture->texts, fixture->clone->path, &result) == SOURCE_EDITS_OK);
}
/** Retire expected/result users before original source/parser/closure ownership. */
static void module_write_fixture_destroy(ModuleWriteFixture *fixture, bool standard) {
    source_text_edits_destroy(&fixture->main_edits); source_file_map_destroy(&fixture->files);
    source_copy_context_omissions_destroy(&fixture->contexts); source_clone_map_destroy(&fixture->mains);
    source_observed_image_destroy(&fixture->observed); source_package_environment_destroy(&fixture->original);
    source_manager_destroy(&fixture->texts); source_arena_destroy(&fixture->arena); finish_copy();
    assert(unlink(standard ? "generated/standard/plain.ini" : "generated/project/packages/vendor/pkg/v1.0.0/src/main.ini") == 0);
}
