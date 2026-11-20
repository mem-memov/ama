/**
 * @file
 * @brief Check coherent whole-version selection independently of module loading.
 *
 * Test-owned physical trees deliberately contain invalid module/configuration
 * bytes to prove this preparation boundary does not parse selected or ignored
 * version contents. Counted allocation checks cover every measured failure point.
 * @see [T231 contract](../../../../../docs/implementation/versioned-source.md)
 */
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../../common.c"
#include "source/module/packages/packages.h"
#include "support/io.h"

/** Own a catalog and its counted allocator for one independent selection call. */
static ModuleCatalog catalog;

/** Initialize the real pre-index owner and reset all fixture allocation counters. */
static void setup_catalog(void) {
    setup_test();
    module_catalog_init(&catalog, sources.allocator);
}

/** Destroy complete or failed discovery storage and require zero retained allocations. */
static void destroy_catalog(void) {
    module_catalog_destroy(&catalog);
    destroy_test();
}

/** Make a bounded known fixture path's directory ancestors, preserving earlier fixtures. */
static void directories(const char *path) {
    char copy[1024];
    size_t length = strlen(path);
    assert(length < sizeof(copy));
    memcpy(copy, path, length + 1);
    for (size_t index = 1; index <= length; index++) {
        if (copy[index] == '/' || copy[index] == 0) {
            char separator = copy[index];
            copy[index] = 0;
            int made = mkdir(copy, 0700);
            assert(made == 0 || errno == EEXIST);
            copy[index] = separator;
        }
    }
}

/** Write exactly one fixture byte so UTF-8-invalid contents cannot be parsed unnoticed. */
static void invalid_content(const char *path) {
    FILE *stream = fopen(path, "wb");
    assert(stream != NULL);
    int written = fputc(255, stream);
    int closed = fclose(stream);
    assert(written == 255 && closed == 0);
}

/** Empty/dependency-only projects and standard roots do not invent local versions. */
static void it_selects_optional_storage_and_exact_sorted_version_prefixes(void) {
    directories("generated/empty");
    setup_catalog();
    ModuleRoot empty = {.path = "generated/empty", .kind = MODULE_ROOT_PROJECT};
    ModulePackage *selected = NULL;
    ModuleStatus status = module_packages_select(&catalog, &empty, 1, &selected);
    assert(status == MODULE_OK && selected == NULL && catalog.first == NULL);
    destroy_catalog();

    directories("generated/external/dependencies/acme/deep/package/v2.0.0-speed");
    directories("generated/external/dependencies/acme/deep/package/v10.0.0");
    invalid_content("generated/external/dependencies/acme/deep/package/v2.0.0-speed/package.ini");
    invalid_content("generated/external/dependencies/acme/deep/package/v2.0.0-speed/bad source");
    setup_catalog();
    ModuleRoot root = {.path = "generated/external", .kind = MODULE_ROOT_PROJECT};
    status = module_packages_select(&catalog, &root, 1, &selected);
    assert(status == MODULE_OK && selected != NULL && !selected->local);
    assert(strcmp(selected->logical, "/acme/deep/package/v10.0.0") == 0);
    assert(strcmp(selected->relative, "dependencies/acme/deep/package/v10.0.0") == 0);
    assert(selected->root == &root && selected->next != NULL);
    assert(strcmp(selected->next->logical, "/acme/deep/package/v2.0.0-speed") == 0);
    assert(selected->next->next == NULL && catalog.first == NULL);
    destroy_catalog();

    setup_catalog();
    ModuleRoot standard = {.path = "not inspected here", .kind = MODULE_ROOT_STANDARD};
    selected = NULL;
    status = module_packages_select(&catalog, &standard, 1, &selected);
    assert(status == MODULE_OK && selected == NULL);
    destroy_catalog();
}

/** Local selection is version-wide across root order, not highest-version selection. */
static void it_selects_local_versions_before_external_roots_in_any_order(void) {
    directories("generated/local/packages/acme/container/v1.2.3");
    directories("generated/remote/dependencies/acme/container/v1.2.3/src");
    directories("generated/remote/dependencies/acme/container/v1.2.4");
    invalid_content("generated/remote/dependencies/acme/container/v1.2.3/src/invalid");
    ModuleRoot roots[] = {
        {.path = "generated/remote", .kind = MODULE_ROOT_PROJECT},
        {.path = "generated/local", .kind = MODULE_ROOT_PROJECT}
    };
    for (size_t order = 0; order < 2; order++) {
        setup_catalog();
        ModulePackage *selected = NULL;
        ModuleStatus status = module_packages_select(&catalog, roots, 2, &selected);
        assert(status == MODULE_OK && selected != NULL && selected->local);
        assert(strcmp(selected->physical, "generated/local/packages/acme/container/v1.2.3") == 0);
        assert(selected->next != NULL && !selected->next->local);
        assert(strcmp(selected->next->logical, "/acme/container/v1.2.4") == 0);
        assert(selected->next->next == NULL && catalog.first == NULL);
        destroy_catalog();
        ModuleRoot temporary = roots[0];
        roots[0] = roots[1];
        roots[1] = temporary;
    }
}

/** Competing same-priority versions cannot merge even disjoint module sets. */
static void it_refuses_competing_versions_without_publishing_partial_selection(void) {
    directories("generated/duplicate-one/packages/acme/p/v1.0.0");
    directories("generated/duplicate-two/packages/acme/p/v1.0.0");
    ModuleRoot roots[] = {
        {.path = "generated/duplicate-one", .kind = MODULE_ROOT_PROJECT},
        {.path = "generated/duplicate-two", .kind = MODULE_ROOT_PROJECT}
    };
    for (size_t order = 0; order < 2; order++) {
        setup_catalog();
        ModulePackage sentinel = {0};
        ModulePackage *selected = &sentinel;
        ModuleStatus status = module_packages_select(&catalog, roots, 2, &selected);
        assert(status == MODULE_AMBIGUOUS && selected == &sentinel);
        assert(catalog.issue.path != NULL && catalog.issue.related_path != NULL);
        assert(strcmp(catalog.issue.path, catalog.issue.related_path) != 0);
        assert(catalog.first == NULL);
        destroy_catalog();
        ModuleRoot temporary = roots[0];
        roots[0] = roots[1];
        roots[1] = temporary;
    }
}

/** External versions also conflict rather than combining disjoint source copies. */
static void it_refuses_two_external_copies_of_the_same_version(void) {
    directories("generated/external-one/dependencies/acme/p/v1.0.0");
    directories("generated/external-two/dependencies/acme/p/v1.0.0");
    ModuleRoot roots[] = {
        {.path = "generated/external-one", .kind = MODULE_ROOT_PROJECT},
        {.path = "generated/external-two", .kind = MODULE_ROOT_PROJECT}
    };
    setup_catalog();
    ModulePackage sentinel = {0};
    ModulePackage *selected = &sentinel;
    ModuleStatus status = module_packages_select(&catalog, roots, 2, &selected);
    assert(status == MODULE_AMBIGUOUS && selected == &sentinel && catalog.first == NULL);
    destroy_catalog();
}

/** Missing roots, symlinks and malformed physical names remain distinct failures. */
static void it_retains_exact_filesystem_failure_context(void) {
    setup_catalog();
    ModuleRoot missing = {.path = "generated/absent", .kind = MODULE_ROOT_PROJECT};
    ModulePackage sentinel = {0};
    ModulePackage *selected = &sentinel;
    ModuleStatus status = module_packages_select(&catalog, &missing, 1, &selected);
    assert(status == MODULE_MISSING && selected == &sentinel && catalog.issue.path != NULL);
    destroy_catalog();

    directories("generated/linked/packages/acme/p");
    int linked = symlink("../../../empty", "generated/linked/packages/acme/p/v1.0.0");
    assert(linked == 0);
    setup_catalog();
    ModuleRoot root = {.path = "generated/linked", .kind = MODULE_ROOT_PROJECT};
    status = module_packages_select(&catalog, &root, 1, &selected);
    assert(status == MODULE_UNSUPPORTED_PATH && selected == &sentinel);
    assert(strcmp(catalog.issue.path, "generated/linked/packages/acme/p/v1.0.0") == 0);
    destroy_catalog();

    directories("generated/bad-name/packages/acme/p");
    directories("generated/bad-name/packages/acme/p/\xff");
    setup_catalog();
    root.path = "generated/bad-name";
    status = module_packages_select(&catalog, &root, 1, &selected);
    assert(status == MODULE_UNSUPPORTED_PATH && selected == &sentinel);
    destroy_catalog();
}

/** A version boundary cannot stand in for a vendor or the required first package. */
static void it_refuses_misplaced_versions_storage_files_and_root_aliases(void) {
    directories("generated/empty");
    const char *paths[] = {"generated/invalid-vendor", "generated/invalid-package"};
    directories("generated/invalid-vendor/packages/v1.0.0");
    directories("generated/invalid-package/packages/acme/v1.0.0");
    for (size_t index = 0; index < 2; index++) {
        setup_catalog();
        ModuleRoot root = {.path = paths[index], .kind = MODULE_ROOT_PROJECT};
        ModulePackage sentinel = {0};
        ModulePackage *selected = &sentinel;
        ModuleStatus status = module_packages_select(&catalog, &root, 1, &selected);
        assert(status == MODULE_INVALID_PATH && selected == &sentinel);
        destroy_catalog();
    }
    directories("generated/storage-file");
    invalid_content("generated/storage-file/packages");
    setup_catalog();
    ModuleRoot bad_storage = {.path = "generated/storage-file", .kind = MODULE_ROOT_PROJECT};
    ModulePackage *selected = NULL;
    ModuleStatus status = module_packages_select(&catalog, &bad_storage, 1, &selected);
    assert(status == MODULE_UNSUPPORTED_PATH);
    destroy_catalog();

    ModuleRoot roots[] = {
        {.path = "generated/empty", .kind = MODULE_ROOT_PROJECT},
        {.path = "generated/empty", .kind = MODULE_ROOT_PROJECT}
    };
    setup_catalog();
    status = module_packages_select(&catalog, roots, 2, &selected);
    assert(status == MODULE_FILE_ALIAS);
    destroy_catalog();
}

/** Every allocation in a measured complete scan can fail without leaking or publishing. */
static void it_unwinds_every_version_selection_allocation_failure(void) {
    directories("generated/allocation/dependencies/acme/p/v1.0.0");
    directories("generated/allocation/dependencies/acme/p/v2.0.0");
    ModuleRoot root = {.path = "generated/allocation", .kind = MODULE_ROOT_PROJECT};
    setup_catalog();
    ModulePackage *selected = NULL;
    ModuleStatus status = module_packages_select(&catalog, &root, 1, &selected);
    assert(status == MODULE_OK);
    size_t allocations = allocation_count;
    destroy_catalog();
    assert(allocations != 0);
    for (size_t point = 1; point <= allocations; point++) {
        setup_catalog();
        failure_at = point;
        ModulePackage sentinel = {0};
        selected = &sentinel;
        status = module_packages_select(&catalog, &root, 1, &selected);
        assert(status == MODULE_OUT_OF_MEMORY && selected == &sentinel);
        assert(catalog.first == NULL);
        assert(catalog.issue.path == NULL || catalog.issue.path != root.path);
        destroy_catalog();
    }
}

/** Inject at every measured directory operation and prove output/stream cleanup. */
static void it_reports_every_measured_directory_io_failure_without_leaking(void) {
    directories("generated/io/packages/acme/p/v1.0.0");
    directories("generated/io/dependencies/acme/p/v2.0.0");
    ModuleRoot root = {.path = "generated/io", .kind = MODULE_ROOT_PROJECT};
    setup_catalog();
    ModulePackage *selected = NULL;
    fixture_io_begin(0);
    ModuleStatus status = module_packages_select(&catalog, &root, 1, &selected);
    size_t operations = fixture_io_end();
    assert(status == MODULE_OK && selected != NULL && operations != 0);
    destroy_catalog();
    for (size_t point = 1; point <= operations; point++) {
        setup_catalog();
        ModulePackage sentinel = {0};
        selected = &sentinel;
        fixture_io_begin(point);
        status = module_packages_select(&catalog, &root, 1, &selected);
        size_t attempted = fixture_io_end();
        assert(attempted >= point && status == MODULE_IO_ERROR && selected == &sentinel);
        assert(catalog.first == NULL && catalog.issue.path != NULL);
        destroy_catalog();
    }
}

/** Reject invalid descriptors before probing roots or changing an output owner. */
static void it_checks_descriptor_preconditions_and_empty_root_sets(void) {
    directories("generated/empty");
    setup_catalog();
    ModulePackage sentinel = {0};
    ModulePackage *selected = &sentinel;
    ModuleStatus status = module_packages_select(NULL, NULL, 0, &selected);
    assert(status == MODULE_INVALID_PATH && selected == &sentinel);
    status = module_packages_select(&catalog, NULL, 1, &selected);
    assert(status == MODULE_INVALID_PATH && selected == &sentinel);
    status = module_packages_select(&catalog, NULL, 0, NULL);
    assert(status == MODULE_INVALID_PATH && selected == &sentinel);
    ModuleRoot invalid = {.path = "not probed", .kind = 0};
    status = module_packages_select(&catalog, &invalid, 1, &selected);
    assert(status == MODULE_INVALID_PATH && selected == &sentinel);
    status = module_packages_select(&catalog, NULL, 0, &selected);
    assert(status == MODULE_OK && selected == NULL);
    catalog.built = true;
    selected = &sentinel;
    status = module_packages_select(&catalog, NULL, 0, &selected);
    assert(status == MODULE_INVALID_PATH && selected == &sentinel);
    catalog.built = false;
    ModuleSource occupied = {0};
    catalog.first = &occupied;
    status = module_packages_select(&catalog, NULL, 0, &selected);
    assert(status == MODULE_INVALID_PATH && selected == &sentinel);
    catalog.first = NULL;
    destroy_catalog();

    setup_catalog();
    ModuleRoot empty = {.path = "generated/empty", .kind = MODULE_ROOT_PROJECT};
    status = module_packages_select(&catalog, &empty, 1, &selected);
    assert(status == MODULE_OK && selected == NULL);
    selected = &sentinel;
    status = module_packages_select(&catalog, &empty, 1, &selected);
    assert(status == MODULE_INVALID_PATH && selected == &sentinel);
    destroy_catalog();
}

/** Register component scenarios explicitly, with setup dependencies visible in their fixtures. */
void run(void) {
    it_selects_optional_storage_and_exact_sorted_version_prefixes();
    it_selects_local_versions_before_external_roots_in_any_order();
    it_refuses_competing_versions_without_publishing_partial_selection();
    it_refuses_two_external_copies_of_the_same_version();
    it_retains_exact_filesystem_failure_context();
    it_refuses_misplaced_versions_storage_files_and_root_aliases();
    it_unwinds_every_version_selection_allocation_failure();
    it_reports_every_measured_directory_io_failure_without_leaking();
    it_checks_descriptor_preconditions_and_empty_root_sets();
}

/** Execute independent selection assertions; the recursive leaf owns output cleanup. */
int main(void) {
    run();
    return 0;
}
