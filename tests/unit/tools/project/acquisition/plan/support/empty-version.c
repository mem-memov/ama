/** @file
 * @brief Prepare and capture real empty versions without inventing executable modules.
 * @see [T235 contract](../../../../../../../docs/implementation/acquisition-plan.md#t235-structural-source-and-test-delivery)
 */
/** Initialize isolated selected-version metadata after its fixture files exist. */
static void prepare_version_owner(const char *physical, const char *const *required, size_t required_count) {
    setup_frontend(); source_arena_init(&facts, sources.allocator);
    acquisition_closure_init(&closure, sources.allocator);
    ModuleRoot root = {physical, NULL, 0, MODULE_ROOT_PROJECT};
    ModuleStatus cataloged = module_catalog_build(&frontend.project.catalog, &root, 1); assert(cataloged == MODULE_OK);
    bool checked = compiler_frontend_check_snapshot(&frontend, required, required_count); assert(checked);
    const ModuleVersion *version = frontend.project.catalog.versions; assert(version && !version->next);
    prepared = (ModulePackage){.logical = version->logical_path, .physical = version->physical_path, .local = true};
    acquisition_sources_init(&origins, sources.allocator, &prepared);
    AcquisitionSourceRequest request = {.local = &prepared}; AcquisitionSourcesIssue issue;
    AcquisitionSourcesStatus bound = acquisition_sources_bind(&origins, &request, &origin, &issue); assert(bound == ACQUISITION_SOURCES_OK);
}
/** Prepare an actual selected empty version without creating a fake entry module. */
static void prepare_empty(void) {
    expansions = admissions = 0; checked_tests = NULL;
    fixture_package_prepare("generated/empty-project", "fixture");
    prepare_version_owner("generated/empty-project", NULL, 0);
}

/** Capture the real empty src tree and version configuration as version facts. */
static AcquisitionClosureStatus expand_empty(void *context, const char *identity, AcquisitionExpansion *output, SourceIssue *issue) {
    (void)context; (void)issue; expansions++; assert(!strcmp(identity, origin->version));
    const ModuleVersion *version = frontend.project.catalog.versions;
    AcquisitionUnitEntry *configuration = source_arena_allocate(&facts, sizeof(*configuration)); assert(configuration);
    *configuration = (AcquisitionUnitEntry){.file = {.path = source_path(version->configuration),
        .content = version->configuration, .state = DELIVERY_FILE_PRESENT, .issue = PATH_FOUND}};
    *output = (AcquisitionExpansion){.source = origin, .configuration = configuration};
    const char *failed = NULL;
    SourceStatus captured = acquisition_directory_capture(&facts, &sources, version->physical_path, "src", &output->files, &failed);
    assert(captured == SOURCE_OK); return ACQUISITION_CLOSURE_OK;
}
/** Explicit empty versions retain their configuration and empty source directory,
 * without fabricated modules or invented mandatory tests. */
static void it_retains_explicit_empty_versions_without_inventing_module_instances(void) {
    prepare_empty(); AcquisitionClosureProvider provider = {.expand = expand_empty, .admit_test = admit};
    AcquisitionClosureStatus status = acquisition_closure_compute(&closure, &origin->version, 1, &provider);
    assert(status == ACQUISITION_CLOSURE_OK && closure.valid && expansions == 1 && admissions == 0 && !closure.tests);
    assert(closure.roles.records && !closure.roles.records->next && closure.roles.records->expanded);
    size_t count = 0;
    for (const AcquisitionMember *member = closure.members.first; member; member = member->next) {
        count++; assert(member->causes && member->causes->source == origin);
    }
    assert(count == 2); finish();
}
