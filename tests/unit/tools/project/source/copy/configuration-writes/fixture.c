/** @file
 * @brief Join real permission-copy inputs with complete P and genuine new-version initialization facts.
 * @see [T276 resumed review](../../../../../../../docs/implementation/source-operations.md#t276-resumed-entry-review-after-t285)
 */
#include "../../../../../../../tools/project/source/operation/writes/configuration/configuration.h"
/** Borrow the enclosing permission fixture while owning exact unit/test mappings. */
typedef struct {
    SourceFileMap files;
    SourceCopyContextOmissions contexts;
    SourceVersionInitialization initialized;
    SourceOperationConfigurationWriteInputs inputs;
} ConfigurationWriteFixture;
/** Complete raw/test provenance designates the actual destination version. */
static void configuration_write_fixture(ConfigurationWriteFixture *result, PermissionAdditionFixture *fixture) {
    memset(result, 0, sizeof(*result)); source_file_map_init(&result->files, sources.allocator);
    source_copy_context_omissions_init(&result->contexts, sources.allocator, &copy, &fixture->map);
    SourceCopyUnitsIssue units;
    assert(source_copy_map_unit_members(&result->files, &copy, &fixture->map, &result->contexts, &units) == SOURCE_COPY_UNITS_OK);
    assert(source_copy_context_omissions_seal(&result->contexts) == SOURCE_COPY_CONTEXT_OK);
    SourceVersionPlacement version = {fixture->target_identity, fixture->target_physical}; SourceCopyTestsIssue tests;
    assert(source_copy_map_test_members(&result->files, &copy, &fixture->map, &version, 1, &tests) == SOURCE_COPY_TESTS_OK);
    assert(source_file_map_seal(&result->files) == SOURCE_FILE_MAP_OK);
    result->inputs = (SourceOperationConfigurationWriteInputs){
        .kind = fixture->new_version ? SOURCE_OPERATION_CONFIGURATION_INITIALIZED : SOURCE_OPERATION_CONFIGURATION_EXISTING,
        .copy = &copy, .mains = &fixture->map, .files = &result->files, .observed = &fixture->observed,
        .preview = &fixture->final, .after = fixture->scope};
    if (fixture->new_version) {
        const SourcePackageScope *source = environment_scope(&fixture->original, "/vendor/pkg/v1.0.0");
        const SourceCatalogImageEntry *mode = NULL;
        for (size_t i = 0; i < fixture->observed.image.count; i++) {
            if (!strcmp(fixture->observed.image.entries[i].path, source_path(source->document.source))) { mode = &fixture->observed.image.entries[i]; }
        }
        assert(mode); SourceVersionInitializationIssue issue;
        assert(source_version_initialize(&fixture->arena, &sources, &fixture->observed, capture.roots, capture.root_count,
            &version, &result->files, mode, NULL, 0, &result->initialized, &issue) == SOURCE_VERSION_INITIALIZATION_OK);
        result->inputs.initialization = &result->initialized;
    } else {
        result->inputs.original = &fixture->original;
        result->inputs.before = environment_scope(&fixture->original, fixture->target_identity);
    }
}
/** The recipe borrows these maps and retires before this fixture. */
static void configuration_write_fixture_destroy(ConfigurationWriteFixture *fixture) {
    source_file_map_destroy(&fixture->files); source_copy_context_omissions_destroy(&fixture->contexts);
}
