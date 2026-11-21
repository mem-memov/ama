/** @file
 * @brief Consume the actual pre-index package selector to prove remote hole filling is impossible.
 * @see [T234 contract](../../../../../../../docs/implementation/acquisition-plan.md)
 */
/** Create only fixture directory ancestors; selected versions intentionally lack inputs. */
static void directories(const char *path) {
    char copy[512]; size_t length = strlen(path); assert(length < sizeof(copy)); memcpy(copy, path, length + 1);
    for (size_t index = 1; index <= length; index++) {
        if (copy[index] && copy[index] != '/') { continue; }
        char saved = copy[index]; copy[index] = 0;
        int made = mkdir(copy, 0700); assert(!made || errno == EEXIST); copy[index] = saved;
    }
}
/** Incomplete selected versions win before config/module availability or any remote scope. */
static void it_uses_actual_prepared_whole_versions_before_routing_or_source_indexing(void) {
    for (int maintained = 0; maintained < 2; maintained++) {
        begin();
        const char *base = maintained ? "generated/view" : "generated/dependency-only";
        char dependency[256], local[256], expected[256];
        (void)snprintf(dependency, sizeof(dependency), "%s/dependencies/a/p/v1.0.0", base);
        (void)snprintf(local, sizeof(local), "%s/packages/a/p/v1.0.0", base);
        directories(dependency);
        if (maintained) { directories(local); }
        ModuleCatalog catalog; module_catalog_init(&catalog, sources.allocator);
        ModuleRoot root = {.path = base, .kind = MODULE_ROOT_PROJECT};
        ModulePackage *prepared = NULL;
        ModuleStatus selected = module_packages_select(&catalog, &root, 1, &prepared);
        assert(selected == MODULE_OK && prepared && prepared->local == (bool)maintained && !prepared->next);
        acquisition_sources_init(&plan, sources.allocator, prepared);
        AcquisitionSourceRequest value = request(NULL, "/a/p/v1.0.0/src/missing");
        assert(value.local == prepared && !value.snapshot && !value.location && !value.resolve_snapshot && !plan.snapshot_count);
        const AcquisitionVersionSource *bound = NULL;
        AcquisitionSourcesStatus status = acquisition_sources_bind(&plan, &value, &bound, &problem);
        assert(status == ACQUISITION_SOURCES_OK && bound->local == prepared && !bound->snapshot);
        (void)snprintf(expected, sizeof(expected), "%s/%s/a/p/v1.0.0", base, maintained ? "packages" : "dependencies");
        assert(!strcmp(bound->directory, expected));
        ModuleStatus indexed = module_package_index(&catalog, prepared);
        assert(indexed != MODULE_OK && !plan.snapshot_count && plan.version_count == 1);
        acquisition_sources_destroy(&plan); module_catalog_destroy(&catalog); finish();
    }
}
