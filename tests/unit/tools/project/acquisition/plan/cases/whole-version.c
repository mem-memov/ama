/** @file
 * @brief Verify whole-version source enumeration composes with mandatory independent tests.
 * @see [T235 contract](../../../../../../../docs/implementation/acquisition-plan.md#t235-structural-source-and-test-delivery)
 */
static const char *const whole_main = "/test/fixture/v1.0.0/src/main";
/** Prepare exact version/source/test facts using the same capture components as
 * module selection, without linking a package-wide executable. */
static AcquisitionClosureStatus expand_whole_version(void *context, const char *identity, AcquisitionExpansion *output, SourceIssue *issue) {
    (void)context; (void)issue; expansions++;
    const ModuleVersion *version = frontend.project.catalog.versions;
    AcquisitionUnitEntry *configuration = source_arena_allocate(&facts, sizeof(*configuration)); assert(configuration);
    *configuration = (AcquisitionUnitEntry){.file = {.path = source_path(version->configuration),
        .content = version->configuration, .state = DELIVERY_FILE_PRESENT, .issue = PATH_FOUND}};
    *output = (AcquisitionExpansion){.source = origin, .configuration = configuration, .definitions = definitions};
    SourceStatus captured; const char *failed = NULL;
    if (!strcmp(identity, origin->version)) {
        captured = acquisition_directory_capture(&facts, &sources, version->physical_path, "src", &output->files, &failed);
        assert(captured == SOURCE_OK); output->submodules = &whole_main; output->submodule_count = 1;
        return ACQUISITION_CLOSURE_OK;
    }
    output->module = module_source(identity);
    captured = capture_unit(output->module, ACQUISITION_ACTIVE, &output->files); assert(captured == SOURCE_OK);
    if (!strcmp(identity, whole_main)) {
        AcquisitionTestDirectory *directory = source_arena_allocate(&facts, sizeof(*directory)); assert(directory);
        captured = acquisition_test_directory(&facts, output->module, directory); assert(captured == SOURCE_OK);
        output->test_directory = directory;
        char *path = source_arena_copy(&facts, directory->physical); assert(path);
        char *slash = strrchr(path, '/'); assert(slash); *slash = 0;
        captured = acquisition_directory_capture(&facts, &sources, path, slash + 1, &output->test_files, &failed);
        assert(captured == SOURCE_OK);
    }
    return ACQUISITION_CLOSURE_OK;
}
/** Duplicate whole-version selection expands one version, its source main and
 * its independently checked test exactly once and keeps every required file. */
static void it_closes_whole_versions_through_their_actual_source_and_test_units(void) {
    expansions = admissions = 0; checked_tests = NULL;
    fixture_package_prepare("generated/whole-version", "fixture");
    directory("generated/whole-version/packages/test/fixture/v1.0.0/tests/main");
    file("generated/whole-version/packages/test/fixture/v1.0.0/src/main", "@!\n");
    file("generated/whole-version/packages/test/fixture/v1.0.0/tests/main/check",
        "@!\ninstance\n/test/fixture/v1.0.0/src/main\ninstance !\n");
    const char *required[] = {whole_main, "/test/fixture/v1.0.0/tests/main/check"};
    prepare_version_owner("generated/whole-version", required, 2);
    SourceStatus exported = delivery_source_export(&facts, &frontend.static_inventory, &definitions); assert(exported == SOURCE_OK);
    const char *seeds[] = {origin->version, origin->version};
    AcquisitionClosureProvider provider = {.context = "generated/whole-version", .expand = expand_whole_version, .admit_test = admit};
    AcquisitionClosureStatus status = acquisition_closure_compute(&closure, seeds, 2, &provider);
    assert(status == ACQUISITION_CLOSURE_OK && closure.valid && expansions == 3 && admissions == 1);
    size_t count = 0;
    for (const AcquisitionMember *member = closure.members.first; member; member = member->next) {
        count++; assert(member->causes && member->causes->source == origin);
    }
    assert(count == 5 && closure.tests && !closure.tests->next); finish();
}
