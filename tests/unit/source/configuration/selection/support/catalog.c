/**
 * @file
 * @brief Compose effective package selection, real source indexing and physical test provenance.
 * Contract: [selection](../../../../../../docs/implementation/project-configuration.md#t262-selection-and-explicit-edge-contract).
 */
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>
#include "source/module/packages/packages.h"

/** Create only known bounded fixture directories; production paths have no such cap. */
static void fixture_directories(const char *name) {
    char value[512];
    assert(strlen(name) < sizeof(value));
    strcpy(value, name);
    for (size_t offset = 1; offset <= strlen(name); offset++) {
        if (value[offset] != '/' && value[offset] != 0) { continue; }
        char saved = value[offset]; value[offset] = 0;
        assert(mkdir(value, 0700) == 0 || errno == EEXIST);
        value[offset] = saved;
    }
}
/** Store empty valid UTF-8 draft bytes; this boundary indexes sources without parsing bodies. */
static void fixture_source(const char *name) {
    FILE *stream = fopen(name, "wb");
    assert(stream != NULL && fclose(stream) == 0);
}

/** Real plus directories cannot masquerade as mirrored test directories after logical erasure. */
static void it_composes_real_selected_catalog_paths_with_physical_test_coverage(void) {
    fixture_directories("generated/layout/packages/acme/pkg/v1.0.0/src/a+");
    fixture_directories("generated/layout/packages/acme/pkg/v1.0.0/tests/a/child");
    fixture_directories("generated/layout/packages/acme/pkg/v1.0.0/tests/struct+");
    fixture_directories("generated/layout/dependencies/acme/pkg/v1.0.0/src");
    const char *files[] = {"package.ini", "src/a", "src/a+/child", "src/struct",
        "tests/a/check", "tests/a/child/check", "tests/struct", "tests/struct+/case"};
    for (size_t index = 0; index < sizeof(files) / sizeof(*files); index++) {
        char filename[512];
        assert(snprintf(filename, sizeof(filename), "generated/layout/packages/acme/pkg/v1.0.0/%s", files[index]) > 0);
        fixture_source(filename);
    }
    fixture_source("generated/layout/dependencies/acme/pkg/v1.0.0/src/ignored");
    begin();
    ModuleCatalog catalog;
    module_catalog_init(&catalog, sources.allocator);
    ModuleRoot root = {.path = "generated/layout", .kind = MODULE_ROOT_PROJECT};
    ModulePackage *package = NULL;
    assert(module_packages_select(&catalog, &root, 1, &package) == MODULE_OK);
    assert(package != NULL && package->next == NULL && package->local);
    assert(module_package_index(&catalog, package) == MODULE_OK);
    DependencyViewInput records[16];
    size_t count = 1;
    records[0] = (DependencyViewInput){path(package->logical), 0};
    for (ModuleSource *source = catalog.first; source != NULL; source = source->next) {
        assert(count < sizeof(records) / sizeof(*records));
        const ModulePathValue *identity = path(source->logical_path);
        ModulePathShape shape;
        assert(module_path_describe((const unsigned char *)identity->text, identity->length, &shape));
        const char *physical_name = source_path(source->text);
        size_t prefix = strlen(package->physical);
        const char *area = shape.area == MODULE_PATH_SRC ? "/src/" : "/tests/";
        assert(strncmp(physical_name, package->physical, prefix) == 0);
        assert(strncmp(physical_name + prefix, area, strlen(area)) == 0);
        const char *relative = physical_name + prefix + strlen(area);
        assert(dependency_view_provenance(identity, (const unsigned char *)relative, strlen(relative),
            &records[count++], &issue) == DEPENDENCY_SELECTION_OK);
    }
    assert(count == 8);
    const DependencyView *actual = NULL;
    assert(dependency_view_build(&results, records, count, &actual, &issue) == DEPENDENCY_SELECTION_OK);
    const DependencySelector *selectors[] = {pattern(package->logical)};
    const DependencySelection *selected = NULL;
    assert(dependency_selection_build(&results, actual, records[0].identity, selectors, 1,
        &selected, &issue) == DEPENDENCY_SELECTION_OK);
    module_catalog_destroy(&catalog);
    assert(dependency_selection_covers(selected, path("/acme/pkg/v1.0.0/tests/a/check")));
    assert(dependency_selection_covers(selected, path("/acme/pkg/v1.0.0/tests/a/child/check")));
    assert(!dependency_selection_covers(selected, path("/acme/pkg/v1.0.0/tests/struct/case")));
    assert(!dependency_selection_covers(selected, path("/acme/pkg/v1.0.0/src/ignored")));
    finish();
}
