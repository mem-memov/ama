/** @file
 * @brief Compare physical identity projection with production catalogs and independent layout partitions.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
#include "../../../../../../tools/project/source/mapping/identity.h"
/** Actual common discovery is the independent oracle for ordinary and explicit standard layouts. */
static void it_projects_existing_source_names_exactly_as_common_catalog_discovery(void) {
    for (unsigned standard = 0; standard < 2; standard++) {
        prepare_scope(standard != 0, standard != 0);
        SourceArena names; source_arena_init(&names, sources.allocator);
        for (const SourceCapturedUnit *unit = capture.units; unit; unit = unit->next) {
            SourcePlacedIdentity result = {0};
            assert(source_placement_identity(&names, capture.roots, capture.root_count,
                source_path(unit->source->text), &result) == SOURCE_IDENTITY_OK);
            assert(!strcmp(result.identity, unit->source->logical_path));
        }
        source_arena_destroy(&names); finish();
    }
}
/** Plus is storage only in module directories, never a package/version/area/main spelling repair. */
static void it_decodes_physical_layouts_without_repairing_invalid_or_excluded_components(void) {
    const char *excluded[] = {"packages/vendor/pkg/v1.0.0/src/private"};
    const ModuleRoot roots[] = {{"/project", excluded, 1, MODULE_ROOT_PROJECT}, {"/standard", NULL, 0, MODULE_ROOT_STANDARD}};
    const struct { const char *path, *expected; } cases[] = {
        {"/project/packages/vendor/pkg/v1.0.0/src/parent+/child", "/vendor/pkg/v1.0.0/src/parent/child"},
        {"/project/dependencies/vendor/tools/math/v12.34.56-variant/tests/space name+/nested_module", "/vendor/tools/math/v12.34.56-variant/tests/space name/nested_module"},
        {"/standard/space name+/a-b_c", "//space name/a-b_c"},
        {"/project/packages/vendor+/pkg/v1.0.0/src/child", NULL},
        {"/project/packages/vendor/pkg/v1.0.0+/src/child", NULL},
        {"/project/packages/vendor/pkg/v1.0.0/src+/child", NULL},
        {"/project/packages/vendor/pkg/v1.0.0/src/child+", NULL},
        {"/project/packages/vendor/pkg/v1.0.0/src/parent++/child", NULL},
        {"/project/packages/vendor/pkg/v1.0.0/src/private/child", NULL},
        {"/project/packages/vendor/pkg/v1.0.0/src/privateish/child", "/vendor/pkg/v1.0.0/src/privateish/child"},
        {"/project/packages/vendor/v1.0.0/src/child", NULL},
        {"/standard/file.c", NULL},
    };
    prepare(); SourceArena names; source_arena_init(&names, sources.allocator);
    for (size_t index = 0; index < sizeof(cases) / sizeof(*cases); index++) {
        SourcePlacedIdentity result = {"sentinel", 99};
        SourceIdentityStatus status = source_placement_identity(&names, roots, 2, cases[index].path, &result);
        assert(status == (cases[index].expected ? SOURCE_IDENTITY_OK : SOURCE_IDENTITY_NOT_MODULE));
        if (cases[index].expected) { assert(!strcmp(result.identity, cases[index].expected)); }
        else { assert(!strcmp(result.identity, "sentinel") && result.root == 99); }
    }
    SourcePlacedIdentity result = {"sentinel", 99};
    assert(source_placement_identity(&names, roots, 2, "/projectish/packages/vendor/pkg/v1.0.0/src/child", &result) == SOURCE_IDENTITY_OUTSIDE);
    source_arena_destroy(&names); finish();
}
/** Multiple eligible explicit roots never select a namespace by enumeration order. */
static void it_refuses_ambiguous_root_interpretations_independently_of_order(void) {
    prepare(); SourceArena names; source_arena_init(&names, sources.allocator);
    ModuleRoot roots[] = {{"/project", NULL, 0, MODULE_ROOT_PROJECT},
        {"/project/packages/vendor/pkg/v1.0.0/src", NULL, 0, MODULE_ROOT_STANDARD}};
    const char *path = "/project/packages/vendor/pkg/v1.0.0/src/child";
    SourcePlacedIdentity result = {"sentinel", 99};
    assert(source_placement_identity(&names, roots, 2, path, &result) == SOURCE_IDENTITY_AMBIGUOUS);
    ModuleRoot swap = roots[0]; roots[0] = roots[1]; roots[1] = swap;
    assert(source_placement_identity(&names, roots, 2, path, &result) == SOURCE_IDENTITY_AMBIGUOUS);
    assert(!strcmp(result.identity, "sentinel") && result.root == 99);
    source_arena_destroy(&names); finish();
}
/** Every candidate allocation failure preserves output, including a second eligible interpretation. */
static void it_retires_every_candidate_identity_allocation_and_invalid_input_without_publication(void) {
    const ModuleRoot roots[] = {{"/project", NULL, 0, MODULE_ROOT_PROJECT},
        {"/project/packages/vendor/pkg/v1.0.0/src", NULL, 0, MODULE_ROOT_STANDARD}};
    const char *path = "/project/packages/vendor/pkg/v1.0.0/src/child";
    for (size_t failure = 1; failure <= 2; failure++) {
        prepare(); SourceArena names; source_arena_init(&names, sources.allocator);
        SourcePlacedIdentity result = {"sentinel", 99}; failure_at = allocation_count + failure;
        assert(source_placement_identity(&names, roots, 2, path, &result) == SOURCE_IDENTITY_MEMORY);
        assert(!strcmp(result.identity, "sentinel") && result.root == 99);
        failure_at = 0; source_arena_destroy(&names); finish();
    }
    prepare(); SourceArena names; source_arena_init(&names, sources.allocator);
    SourcePlacedIdentity result = {"sentinel", 99};
    assert(source_placement_identity(NULL, roots, 2, path, &result) == SOURCE_IDENTITY_INVALID);
    assert(source_placement_identity(&names, NULL, 2, path, &result) == SOURCE_IDENTITY_INVALID);
    assert(source_placement_identity(&names, roots, 2, NULL, &result) == SOURCE_IDENTITY_INVALID);
    assert(source_placement_identity(&names, roots, 2, "relative", &result) == SOURCE_IDENTITY_INVALID);
    assert(source_placement_identity(&names, roots, 2, path, NULL) == SOURCE_IDENTITY_INVALID);
    assert(source_placement_identity(&names, NULL, 0, path, &result) == SOURCE_IDENTITY_OUTSIDE);
    source_arena_destroy(&names); finish();
}
