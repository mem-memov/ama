/** @file
 * @brief Check shared catalog availability against actual discovery and every allocation refusal.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
#include "source/configuration/selection/catalog.h"
/** Own a finite local/external/standard fixture; catalog selection supplies the
 * actual universe, including a local version that masks its external counterpart. */
static void catalog_view_fixture(ModuleCatalog *catalog) {
    const char *base = "generated/catalog-view/packages/acme/pkg/v1.0.0/";
    fixture_directories("generated/catalog-view/packages/acme/pkg/v1.0.0/src/a+");
    fixture_directories("generated/catalog-view/packages/acme/pkg/v1.0.0/tests/a");
    fixture_directories("generated/catalog-view/packages/acme/pkg/v1.0.0/tests/struct+");
    fixture_directories("generated/catalog-view/packages/acme/pkg/v2.0.0/src");
    fixture_directories("generated/catalog-view/packages/acme/pkg/v2.0.0/tests");
    fixture_directories("generated/catalog-view/dependencies/acme/pkg/v1.0.0/src");
    fixture_directories("generated/catalog-standard");
    const char *const files[] = {"package.ini", "src/a", "src/a+/child", "tests/a/check", "tests/struct", "tests/struct+/case"};
    for (size_t i = 0; i < sizeof(files) / sizeof(*files); i++) {
        char filename[512]; assert(snprintf(filename, sizeof(filename), "%s%s", base, files[i]) > 0); fixture_source(filename);
    }
    fixture_source("generated/catalog-view/packages/acme/pkg/v2.0.0/package.ini");
    fixture_source("generated/catalog-view/dependencies/acme/pkg/v1.0.0/src/ignored");
    fixture_source("generated/catalog-standard/integer");
    ModuleRoot roots[] = {{.path = "generated/catalog-view", .kind = MODULE_ROOT_PROJECT},
        {.path = "generated/catalog-standard", .kind = MODULE_ROOT_STANDARD}};
    module_catalog_init(catalog, sources.allocator);
    ModuleStatus status = module_catalog_build(catalog, roots, 2);
    if (status != MODULE_OK) { fprintf(stderr, "catalog status %d at %s\n", status, catalog->issue.path ? catalog->issue.path : "<no path>"); }
    assert(status == MODULE_OK);
}
/** Real empty versions, source/test physical ancestry and standard isolation must
 * survive catalog teardown as independent canonical view metadata. */
static void it_builds_shared_dependency_views_from_actual_catalogs_with_owned_lifetimes(void) {
    begin(); ModuleCatalog catalog; catalog_view_fixture(&catalog);
    const DependencyView *view = NULL;
    assert(dependency_catalog_view(&results, &catalog, &view, &issue) == DEPENDENCY_SELECTION_OK);
    assert(view->count == 7);
    const DependencySelector *selectors[] = {pattern("/acme/pkg/v1.0.0")};
    const DependencySelection *selection = NULL;
    assert(dependency_selection_build(&results, view, path("/acme/pkg/v1.0.0"), selectors, 1, &selection, &issue) == DEPENDENCY_SELECTION_OK);
    bool empty_version = false, structural = false, ordinary = false;
    for (size_t i = 0; i < view->count; i++) {
        const DependencyViewEntry *entry = &view->entries[i];
        assert(strncmp(entry->identity->text, "//", 2) && !strstr(entry->identity->text, "ignored"));
        if (!strcmp(entry->identity->text, "/acme/pkg/v2.0.0")) { empty_version = true; }
        if (!strcmp(entry->identity->text, "/acme/pkg/v1.0.0/tests/struct/case")) { structural = true; assert(entry->test_directory_prefix == 0); }
        if (!strcmp(entry->identity->text, "/acme/pkg/v1.0.0/tests/a/check")) { ordinary = true; assert(entry->test_directory_prefix == 1); }
    }
    assert(empty_version && structural && ordinary); module_catalog_destroy(&catalog);
    assert(dependency_selection_covers(selection, path("/acme/pkg/v1.0.0/tests/a/check")));
    assert(!dependency_selection_covers(selection, path("/acme/pkg/v1.0.0/tests/struct/case")));
    assert(!dependency_selection_covers(selection, path("/acme/pkg/v1.0.0/src/ignored")));
    finish();
}
/** Failed scratch remains arena-owned, output is never partially published,
 * and invalid provenance cannot become a logical-depth approximation. */
static void it_refuses_invalid_catalog_views_and_retires_every_preparation_allocation(void) {
    begin(); ModuleCatalog catalog; catalog_view_fixture(&catalog);
    DependencyView sentinel = {0}; const DependencyView *output = &sentinel;
    SourceArena attempt; source_arena_init(&attempt, sources.allocator);
    assert(dependency_catalog_view(&attempt, NULL, &output, &issue) == DEPENDENCY_SELECTION_INVALID && output == &sentinel);
    catalog.built = false;
    assert(dependency_catalog_view(&attempt, &catalog, &output, &issue) == DEPENDENCY_SELECTION_INVALID && output == &sentinel);
    catalog.built = true;
    ModuleSource *test = catalog.first;
    while (test && !strstr(test->logical_path, "/tests/")) { test = test->next; }
    assert(test && test->version);
    ModuleVersion wrong = *test->version; const ModuleVersion *saved = test->version;
    wrong.physical_path = "unrelated/version"; test->version = &wrong;
    assert(dependency_catalog_view(&attempt, &catalog, &output, &issue) == DEPENDENCY_SELECTION_INVALID && output == &sentinel);
    assert(issue.span.source == test->text); test->version = saved;
    source_arena_destroy(&attempt);
    size_t total = 0, baseline = live_allocations;
    for (size_t failure = 0; !failure || failure <= total; failure++) {
        source_arena_init(&attempt, sources.allocator); output = &sentinel;
        size_t before = allocation_count; if (failure) { failure_at = before + failure; }
        DependencySelectionStatus status = dependency_catalog_view(&attempt, &catalog, &output, &issue);
        if (!failure) { assert(status == DEPENDENCY_SELECTION_OK && output != &sentinel); total = allocation_count - before; assert(total); }
        else { assert(status == DEPENDENCY_SELECTION_OUT_OF_MEMORY && output == &sentinel); }
        failure_at = 0; source_arena_destroy(&attempt); assert(live_allocations == baseline);
    }
    module_catalog_destroy(&catalog); finish();
    begin(); module_catalog_init(&catalog, sources.allocator);
    ModuleRoot standard = {.path = "generated/catalog-standard", .kind = MODULE_ROOT_STANDARD};
    assert(module_catalog_build(&catalog, &standard, 1) == MODULE_OK);
    output = NULL; assert(dependency_catalog_view(&results, &catalog, &output, &issue) == DEPENDENCY_SELECTION_OK && !output->count);
    module_catalog_destroy(&catalog); finish();
}
