/**
 * @file
 * @brief Check exact physical/logical roots, module catalog identity and filesystem failures.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../docs/implementation/library-tests-and-build.md)
 */
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include "../common.c"
#include "source/module/catalog.h"

/**
 * Create a test-owned fixture directory or accept its existing directory setup as specified by
 * the caller; other mkdir errors fail assertions.
 */
static void directory(const char *path) {
    int result = mkdir(path, 0700);
    assert(result == 0 || errno == EEXIST);
}

/**
 * Write exact supplied source text into a fixture path, checking full byte count and close
 * before loading it.
 */
static void file(const char *path, const char *content) {
    FILE *stream = fopen(path, "wb");
    assert(stream != NULL);
    size_t length = strlen(content);
    size_t written = fwrite(content, 1, length, stream);
    int closed = fclose(stream);
    assert(written == length && closed == 0);
}

/** Create one complete local test package before catalog discovery; roots remain
 * explicit project roots rather than silently treating an arbitrary directory
 * as a source area. Both required areas exist even when one has no modules.
 */
static void prepare_version(const char *root, const char *storage, const char *version) {
    directory(root);
    char path[1024];
    const char *suffixes[] = {"", "/test", "/test/fixture"};
    for (size_t index = 0; index < 3; index++) {
        int written = snprintf(path, sizeof(path), "%s/%s%s", root, storage, suffixes[index]);
        assert(written > 0 && (size_t)written < sizeof(path));
        directory(path);
    }
    const char *areas[] = {"", "/src", "/tests"};
    for (size_t index = 0; index < 3; index++) {
        int written = snprintf(path, sizeof(path), "%s/%s/test/fixture/%s%s",
            root, storage, version, areas[index]);
        assert(written > 0 && (size_t)written < sizeof(path));
        directory(path);
    }
    int written = snprintf(path, sizeof(path), "%s/%s/test/fixture/%s/package.ini", root, storage, version);
    assert(written > 0 && (size_t)written < sizeof(path));
    file(path, "");
}

/** Prepare the same explicit local version used by ordinary catalog fixture keys. */
static void prepare_package(const char *root) {
    prepare_version(root, "packages", "v1.0.0");
}

static ModuleCatalog catalog;

/**
 * Initialize a counted exact module catalog and physical fixture ownership before indexing.
 */
static void setup_catalog(void) {
    setup_test();
    module_catalog_init(&catalog, sources.allocator);
}

/**
 * Destroy catalog/source records before allocator teardown and assert no allocation remains.
 */
static void destroy_catalog(void) {
    module_catalog_destroy(&catalog);
    destroy_test();
}

#include "support/roots.c"
#include "support/storage.c"
#include "support/enumeration.c"
#include "support/packages.c"

/**
 * Write this scenario's explicit source/native fixtures in test-owned storage before loading or
 * compiling them; the caller controls their lifetime and expected result.
 */
static void prepare_project(void) {
    prepare_package("generated/project");
    directory("generated/project/packages/test/fixture/v1.0.0/src/shop");
    file("generated/project/packages/test/fixture/v1.0.0/src/shop/item", "unfinished parent is allowed here");
    directory("generated/project/packages/test/fixture/v1.0.0/src/shop/item+");
    file("generated/project/packages/test/fixture/v1.0.0/src/shop/item+/验证", "@C\r\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/shop/item+/验证.c", "/* C */");
    file("generated/project/packages/test/fixture/v1.0.0/src/shop/item+/验证.h", "/* header */");
    file("generated/project/packages/test/fixture/v1.0.0/src/shop/item+/验证.abi", "+ construct\n- destroy\n");
    directory("generated/project/packages/test/fixture/v1.0.0/src/shop/item+/验证+");
    file("generated/project/packages/test/fixture/v1.0.0/src/shop/item+/验证+/details", "@!\n");
    directory("generated/project/packages/test/fixture/v1.0.0/src/generated");
    file("generated/project/packages/test/fixture/v1.0.0/src/generated/not_a_source", "\xff");
    file("generated/project/packages/test/fixture/v1.0.0/src/README.md", "not a source");
}

/**
 * Return the explicit fixture root/options used for catalog tests, retaining no unrelated
 * filesystem search policy.
 */
static ModuleRoot project_root(void) {
    static const char *const exclusions[] = {"packages/test/fixture/v1.0.0/src/generated"};
    return (ModuleRoot){"generated/project", exclusions, 1, MODULE_ROOT_PROJECT};
}

/**
 * Assert that the component resolves nested submodules and physical native companions. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_resolves_nested_submodules_and_physical_native_companions(void) {
    prepare_project();
    setup_catalog();
    ModuleRoot root = project_root();
    ModuleStatus status = module_catalog_build(&catalog, &root, 1);
    assert(status == MODULE_OK);
    const ModuleSource *source = NULL;
    status = module_catalog_find(&catalog, "/test/fixture/v1.0.0/src/shop/item/验证", &source);
    assert(status == MODULE_OK);
    assert(strcmp(source_path(source->text), "generated/project/packages/test/fixture/v1.0.0/src/shop/item+/验证") == 0);
    status = module_catalog_load_native(&catalog, source);
    assert(status == MODULE_OK);
    assert(strcmp(source->native_c, "generated/project/packages/test/fixture/v1.0.0/src/shop/item+/验证.c") == 0);
    assert(strcmp(source->native_header, "generated/project/packages/test/fixture/v1.0.0/src/shop/item+/验证.h") == 0);
    assert(strcmp(source_path(source->native_abi), "generated/project/packages/test/fixture/v1.0.0/src/shop/item+/验证.abi") == 0);
    status = module_catalog_find(&catalog, "/test/fixture/v1.0.0/src/shop/item/验证/details", &source);
    assert(status == MODULE_OK);
    status = module_catalog_find(&catalog, "/test/fixture/v1.0.0/src/generated/not_a_source", &source);
    assert(status == MODULE_MISSING);
    status = module_catalog_find(&catalog, "/test/fixture/v1.0.0/src/shop/item/验证/c", &source);
    assert(status == MODULE_MISSING);
    status = module_catalog_find(&catalog, "/test/fixture/v1.0.0/src/shop//item", &source);
    assert(status == MODULE_INVALID_PATH);
    destroy_catalog();
}

/**
 * Assert that the component rejects duplicates across roots instead of preferring one. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_rejects_duplicates_across_roots_instead_of_preferring_one(void) {
    prepare_package("generated/first");
    prepare_package("generated/second");
    file("generated/first/packages/test/fixture/v1.0.0/src/item", "@!\n");
    file("generated/second/packages/test/fixture/v1.0.0/src/item", "@!\n");
    ModuleRoot roots[] = {{"generated/first", NULL, 0, MODULE_ROOT_PROJECT}, {"generated/second", NULL, 0, MODULE_ROOT_PROJECT}};
    for (size_t order = 0; order < 2; order++) {
        setup_catalog();
        ModuleStatus status = module_catalog_build(&catalog, roots, 2);
        assert(status == MODULE_AMBIGUOUS);
        assert(catalog.issue.path != NULL && catalog.issue.related_path != NULL);
        const ModuleSource *source = NULL;
        status = module_catalog_find(&catalog, "/test/fixture/v1.0.0/src/item", &source);
        assert(status == MODULE_INVALID_PATH && source == NULL);
        destroy_catalog();
        ModuleRoot temporary = roots[0];
        roots[0] = roots[1];
        roots[1] = temporary;
    }
}

/**
 * Assert that the component rejects orphans hardlink aliases and symlink roots. The explicit
 * fixture setup and observations define this regression; assertion failure keeps its generated
 * evidence.
 */
static void it_rejects_orphans_hardlink_aliases_and_symlink_roots(void) {
    prepare_package("generated/orphan");
    directory("generated/orphan/packages/test/fixture/v1.0.0/src/item+");
    setup_catalog();
    ModuleRoot root = {"generated/orphan", NULL, 0, MODULE_ROOT_PROJECT};
    ModuleStatus status = module_catalog_build(&catalog, &root, 1);
    assert(status == MODULE_ORPHAN_DIRECTORY);
    destroy_catalog();
    prepare_package("generated/aliases");
    file("generated/aliases/packages/test/fixture/v1.0.0/src/item", "@!\n");
    int removed = unlink("generated/aliases/packages/test/fixture/v1.0.0/src/other");
    assert(removed == 0 || errno == ENOENT);
    int linked = link("generated/aliases/packages/test/fixture/v1.0.0/src/item", "generated/aliases/packages/test/fixture/v1.0.0/src/other");
    assert(linked == 0);
    setup_catalog();
    root.path = "generated/aliases";
    status = module_catalog_build(&catalog, &root, 1);
    assert(status == MODULE_FILE_ALIAS);
    destroy_catalog();
    removed = unlink("generated/shortcut");
    assert(removed == 0 || errno == ENOENT);
    linked = symlink("first", "generated/shortcut");
    assert(linked == 0);
    setup_catalog();
    root.path = "generated/shortcut";
    status = module_catalog_build(&catalog, &root, 1);
    assert(status == MODULE_UNSUPPORTED_PATH);
    destroy_catalog();
}

/**
 * Assert that the component distinguishes case and canonically equivalent source spellings. The
 * explicit fixture setup and observations define this regression; assertion failure keeps its
 * generated evidence.
 */
static void it_distinguishes_case_and_canonically_equivalent_source_spellings(void) {
    prepare_package("generated/spelling");
    const char *names[] = {"generated/spelling/packages/test/fixture/v1.0.0/src/Item", "generated/spelling/packages/test/fixture/v1.0.0/src/item",
        "generated/spelling/packages/test/fixture/v1.0.0/src/é", "generated/spelling/packages/test/fixture/v1.0.0/src/e\xcc\x81"};
    for (size_t index = 0; index < 4; index++) {
        file(names[index], "@!\n");
    }
    setup_catalog();
    ModuleRoot root = {"generated/spelling", NULL, 0, MODULE_ROOT_PROJECT};
    ModuleStatus status = module_catalog_build(&catalog, &root, 1);
    assert(status == MODULE_OK);
    const char *logical[] = {"/test/fixture/v1.0.0/src/Item", "/test/fixture/v1.0.0/src/item", "/test/fixture/v1.0.0/src/é", "/test/fixture/v1.0.0/src/e\xcc\x81"};
    for (size_t index = 0; index < 4; index++) {
        const ModuleSource *source = NULL;
        status = module_catalog_find(&catalog, logical[index], &source);
        assert(status == MODULE_OK && strcmp(source_path(source->text), names[index]) == 0);
    }
    destroy_catalog();
}

/**
 * Assert that the component reports the exact invalid utf8 span in sources and abi. The explicit
 * fixture setup and observations define this regression; assertion failure keeps its generated
 * evidence.
 */
static void it_reports_the_exact_invalid_utf8_span_in_sources_and_abi(void) {
    prepare_package("generated/invalid");
    file("generated/invalid/packages/test/fixture/v1.0.0/src/item", "@!\n\xff");
    ModuleRoot root = {"generated/invalid", NULL, 0, MODULE_ROOT_PROJECT};
    setup_catalog();
    ModuleStatus status = module_catalog_build(&catalog, &root, 1);
    assert(status == MODULE_INVALID_UTF8 && catalog.issue.offset == 3);
    assert(catalog.issue.source != NULL);
    destroy_catalog();
    file("generated/invalid/packages/test/fixture/v1.0.0/src/item", "@C\n");
    file("generated/invalid/packages/test/fixture/v1.0.0/src/item.abi", "+ good\n\xed\xa0\x80");
    setup_catalog();
    status = module_catalog_build(&catalog, &root, 1);
    assert(status == MODULE_OK);
    const ModuleSource *source = NULL;
    status = module_catalog_find(&catalog, "/test/fixture/v1.0.0/src/item", &source);
    assert(status == MODULE_OK);
    status = module_catalog_load_native(&catalog, source);
    assert(status == MODULE_INVALID_UTF8 && catalog.issue.offset == 7);
    assert(strcmp(catalog.issue.path, "generated/invalid/packages/test/fixture/v1.0.0/src/item.abi") == 0);
    destroy_catalog();
}

/**
 * Assert that the component rejects aliased directory roots even when empty. The explicit
 * fixture setup and observations define this regression; assertion failure keeps its generated
 * evidence.
 */
static void it_rejects_aliased_directory_roots_even_when_empty(void) {
    directory("generated/empty");
    ModuleRoot roots[] = {{"generated/empty", NULL, 0, MODULE_ROOT_PROJECT}, {"generated/./empty", NULL, 0, MODULE_ROOT_PROJECT}};
    setup_catalog();
    ModuleStatus status = module_catalog_build(&catalog, roots, 2);
    assert(status == MODULE_FILE_ALIAS);
    destroy_catalog();
}

/**
 * Assert that the component unwinds every catalog allocation failure. The explicit fixture setup
 * and observations define this regression; assertion failure keeps its generated evidence.
 */
static void it_unwinds_every_catalog_allocation_failure(void) {
    prepare_project();
    setup_catalog();
    ModuleRoot root = project_root();
    ModuleStatus status = module_catalog_build(&catalog, &root, 1);
    assert(status == MODULE_OK);
    size_t total = allocation_count;
    destroy_catalog();
    for (size_t failure = 1; failure <= total; failure++) {
        setup_catalog();
        failure_at = failure;
        status = module_catalog_build(&catalog, &root, 1);
        assert(status == MODULE_OUT_OF_MEMORY && !catalog.built);
        destroy_catalog();
    }
}

#include "support/native.c"
#include "support/override.c"
#include "support/snapshot.c"
#include "support/configuration.c"

/**
 * Run every explicitly selected assertion case in this executable; a failed assertion preserves
 * leaf generated output for diagnosis.
 */
void run(void) {
    it_retains_selected_version_configuration_and_exact_module_ownership();
    it_attaches_copied_configuration_to_exact_snapshot_versions();
    it_unwinds_every_configuration_snapshot_allocation_failure();
    it_refuses_invalid_configuration_snapshot_contexts_and_retains_utf8_sites();
    it_reports_selected_configuration_utf8_without_publishing_a_catalog();
    it_indexes_only_selected_versions_and_never_fills_local_module_holes();
    it_reports_missing_selected_package_inputs_without_external_fallback();
    it_does_not_turn_local_exclusions_into_external_or_standard_fallback();
    it_indexes_dependency_only_projects_and_keeps_versions_distinct();
    it_retains_invalid_native_snapshot_diagnostics_and_empty_presence();
    it_owns_native_snapshot_companions_and_unwinds_each_failed_copy();
    it_owns_captured_source_snapshots_without_rereading_files();
    it_retains_captured_registry_collision_and_encoding_diagnostics();
    it_releases_every_failed_captured_registry_prefix();
    it_publishes_native_companions_once_after_explicit_selection();
    it_unwinds_every_native_companion_allocation_without_partial_publication();
    it_rejects_cr_roots_unsupported_by_the_verified_c_target();
    it_enumerates_the_same_catalog_independently_of_file_creation_order();
    it_excludes_only_declared_owned_paths_and_slash_delimited_descendants();
    it_validates_exact_project_and_standard_paths_with_multiword_components();
    it_resolves_explicit_root_families_without_fallback_or_physical_root_names();
    it_rejects_unconfigured_root_kind_and_missing_root_inventory();
    it_maps_nested_multiword_submodules_and_native_companions_in_each_root_kind();
    it_rejects_conflicting_standard_roots_and_cross_family_physical_aliases();
    it_reports_exact_missing_multiword_parent_anchors_without_alternate_resolution();
    it_unwinds_each_multiword_standard_catalog_allocation_failure();
    it_resolves_nested_submodules_and_physical_native_companions();
    it_rejects_duplicates_across_roots_instead_of_preferring_one();
    it_rejects_orphans_hardlink_aliases_and_symlink_roots();
    it_distinguishes_case_and_canonically_equivalent_source_spellings();
    it_reports_the_exact_invalid_utf8_span_in_sources_and_abi();
    it_rejects_aliased_directory_roots_even_when_empty();
    it_unwinds_every_catalog_allocation_failure();
    it_overrides_candidate_text_without_changing_identity_or_old_source_views();
    it_unwinds_every_candidate_source_override_allocation();
}

/**
 * Enter the explicit test run and return success only if all assertions complete; the parent
 * harness observes this process status.
 */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
