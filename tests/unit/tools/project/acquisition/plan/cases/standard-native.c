/** @file
 * @brief Verify standard native inputs preserve companions without ordinary version or test scope.
 * @see [T235 contract](../../../../../../../docs/implementation/acquisition-plan.md#t235-structural-source-and-test-delivery)
 */
/** A standard native definition owns companions but no ordinary package version,
 * configuration, guidance or mandatory associated test directory. */
static AcquisitionClosureStatus expand_standard(void *context, const char *identity, AcquisitionExpansion *output, SourceIssue *issue) {
    (void)context; (void)issue; expansions++;
    const ModuleSource *source = module_source(identity);
    *output = (AcquisitionExpansion){.module = source, .definitions = definitions};
    SourceStatus captured = capture_unit(source, ACQUISITION_ACTIVE, &output->files);
    assert(captured == SOURCE_OK); return ACQUISITION_CLOSURE_OK;
}
/** Compile-time standard native metadata participates without running CMake or C;
 * exact companions remain source inputs with explicit standard-root provenance. */
static void it_retains_standard_native_inputs_without_inventing_package_scope_or_tests(void) {
    directory("generated/standard");
    file("generated/standard/value", "@C\n");
    file("generated/standard/value.abi", "+ initialize\n- destroy\n");
    file("generated/standard/value.h", "#error Must not compile native code during delivery planning\n");
    file("generated/standard/value.c", "#error Must not execute native code during delivery planning\n");
    file("generated/standard/value.cmake", "message(FATAL_ERROR \"Must not execute CMake during delivery planning\")\n");
    file("generated/standard/value.custom", "retain this authored companion\n");
    expansions = admissions = 0; checked_tests = NULL; origin = NULL;
    setup_frontend(); source_arena_init(&facts, sources.allocator);
    acquisition_closure_init(&closure, sources.allocator); acquisition_sources_init(&origins, sources.allocator, NULL);
    ModuleRoot root = {"generated/standard", NULL, 0, MODULE_ROOT_STANDARD};
    ModuleStatus cataloged = module_catalog_build(&frontend.project.catalog, &root, 1); assert(cataloged == MODULE_OK);
    const char *identity = "//value";
    bool checked = compiler_frontend_check_snapshot(&frontend, &identity, 1); assert(checked);
    SourceStatus exported = delivery_source_export(&facts, &frontend.static_inventory, &definitions); assert(exported == SOURCE_OK);
    AcquisitionClosureProvider provider = {.expand = expand_standard, .admit_test = admit};
    AcquisitionClosureStatus status = acquisition_closure_compute(&closure, &identity, 1, &provider);
    assert(status == ACQUISITION_CLOSURE_OK && closure.valid && expansions == 1 && admissions == 0 && !closure.tests);
    size_t count = 0;
    for (const AcquisitionMember *member = closure.members.first; member; member = member->next) {
        count++; assert(member->content && !member->directory && member->causes && !member->causes->source);
    }
    assert(count == 6); finish();
}
