/** @file
 * @brief Compose actual captured/compiled catalogs with independent strict package scopes.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
#include "../../../../../../tools/project/source/environment/environment.h"
/** Configure same-named aliases independently in two actual selected versions. */
static void prepare_environment_catalog(void) {
    setup_test(); fixtures();
    file("generated/project/packages/vendor/pkg/v1.0.0/package.ini", "[aliases]\n~code = ./src\n[dependencies]\n~code/*\n");
    file("generated/project/packages/vendor/pkg/v2.0.0/package.ini", "[aliases]\n~code = ./tests\n[dependencies]\n~code/*\n");
    capture_fixtures();
}
/** Find one actual scope without inferring package membership from paths. */
static const SourcePackageScope *environment_scope(const SourcePackageEnvironment *owner, const char *name) {
    for (const SourcePackageScope *row = owner->first; row; row = row->next) { if (!strcmp(row->version->logical_path, name)) { return row; } }
    assert(false); return NULL;
}
/** Neutral scopes use real compiler inputs and exact source/test provenance;
 * package aliases never leak into the other actual version or standard root. */
static void it_prepares_actual_package_environments_with_exact_version_isolation_and_matches(void) {
    prepare_environment_catalog(); SourcePackageEnvironment owner; source_package_environment_init(&owner, sources.allocator);
    assert(source_package_environment_build(&owner, &capture.catalog) == SOURCE_ENVIRONMENT_OK && owner.valid);
    const SourcePackageScope *first = environment_scope(&owner, "/vendor/pkg/v1.0.0");
    const SourcePackageScope *second = environment_scope(&owner, "/vendor/pkg/v2.0.0");
    assert(first != second && owner.first && owner.first->next && !owner.first->next->next);
    assert(first->document.source == first->version->configuration && second->document.source == second->version->configuration);
    assert(first->aliases->count == 1 && second->aliases->count == 1);
    assert(!strcmp(first->aliases->entries[0].target->text, "/vendor/pkg/v1.0.0/src"));
    assert(!strcmp(second->aliases->entries[0].target->text, "/vendor/pkg/v2.0.0/tests"));
    assert(first->selection->view == owner.view && second->selection->view == owner.view);
    assert(first->selection->count == 4 && !second->selection->count && owner.view->count == 9);
    for (size_t i = 0; i < owner.view->count; i++) {
        const DependencyViewEntry *entry = &owner.view->entries[i];
        if (entry->shape.extent != MODULE_PATH_MODULE) { continue; }
        assert(dependency_selection_covers(first->selection, entry->identity));
        assert(!dependency_selection_covers(second->selection, entry->identity));
    }
    const SourcePackageScope *saved = owner.first; size_t calls = allocation_count;
    assert(source_package_environment_build(&owner, &capture.catalog) == SOURCE_ENVIRONMENT_INVALID);
    assert(owner.valid && owner.first == saved && allocation_count == calls);
    source_package_environment_destroy(&owner); finish();
}
/** Every partial document/result arena must be retired, including failure after
 * a previous version scope has succeeded. No failed owner publishes valid. */
static void it_retires_every_actual_package_environment_preparation_allocation(void) {
    prepare_environment_catalog(); size_t baseline = live_allocations, total = 0;
    for (size_t failure = 0; !failure || failure <= total; failure++) {
        SourcePackageEnvironment owner; source_package_environment_init(&owner, sources.allocator);
        size_t before = allocation_count; if (failure) { failure_at = before + failure; }
        SourceEnvironmentStatus status = source_package_environment_build(&owner, &capture.catalog);
        if (!failure) { assert(status == SOURCE_ENVIRONMENT_OK && owner.valid); total = allocation_count - before; assert(total); }
        else { assert(status == SOURCE_ENVIRONMENT_MEMORY && !owner.valid && owner.status == status); }
        size_t after = allocation_count; assert(source_package_environment_build(&owner, &capture.catalog) == SOURCE_ENVIRONMENT_INVALID);
        assert(allocation_count == after); failure_at = 0;
        source_package_environment_destroy(&owner); assert(live_allocations == baseline);
    }
    SourcePackageEnvironment owner; source_package_environment_init(&owner, sources.allocator);
    assert(source_package_environment_build(&owner, NULL) == SOURCE_ENVIRONMENT_INVALID && !owner.valid);
    source_package_environment_destroy(&owner); finish();
}
/** Catalog discovery retains malformed configuration bytes; the environment
 * rejects them explicitly even for an otherwise unused empty version. */
static void it_refuses_malformed_empty_version_environments_and_keeps_standard_roots_scope_free(void) {
    const char *const malformed[] = {"[unknown]\n", "[aliases]\n~bad = ../../../../../../outside\n", "[dependencies]\n~missing/*\n"};
    const SourceEnvironmentPhase phases[] = {SOURCE_ENVIRONMENT_DOCUMENT, SOURCE_ENVIRONMENT_ALIASES, SOURCE_ENVIRONMENT_SELECTORS};
    for (size_t i = 0; i < sizeof(malformed) / sizeof(*malformed); i++) {
        setup_test(); fixtures(); file("generated/project/packages/vendor/pkg/v2.0.0/package.ini", malformed[i]);
        SourceRootCapture local; source_root_capture_init(&local, sources.allocator);
        ModuleRoot root = {.path = "generated/project", .kind = MODULE_ROOT_PROJECT};
        assert(source_root_capture_build(&local, &root, 1) == SOURCE_CAPTURE_OK);
        SourcePackageEnvironment owner; source_package_environment_init(&owner, sources.allocator);
        assert(source_package_environment_build(&owner, &local.catalog) == SOURCE_ENVIRONMENT_CONFIGURATION && !owner.valid);
        assert(owner.phase == phases[i] && owner.failed_version && !strcmp(owner.failed_version->logical_path, "/vendor/pkg/v2.0.0"));
        assert(owner.issue.span.source == owner.failed_version->configuration);
        source_package_environment_destroy(&owner); source_root_capture_destroy(&local); destroy_test();
    }
    setup_test(); fixtures(); SourceRootCapture local; source_root_capture_init(&local, sources.allocator);
    ModuleRoot root = {.path = "generated/standard", .kind = MODULE_ROOT_STANDARD};
    assert(source_root_capture_build(&local, &root, 1) == SOURCE_CAPTURE_OK);
    SourcePackageEnvironment owner; source_package_environment_init(&owner, sources.allocator);
    assert(source_package_environment_build(&owner, &local.catalog) == SOURCE_ENVIRONMENT_OK && owner.valid && !owner.first && !owner.view->count);
    source_package_environment_destroy(&owner); source_root_capture_destroy(&local); destroy_test();
}
