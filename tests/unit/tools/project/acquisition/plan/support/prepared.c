/** @file
 * @brief Own prepared ordinary source fixtures, checked exports and real capture/admission callbacks.
 * @see [T235 contract](../../../../../../../docs/implementation/acquisition-plan.md#t235-structural-source-and-test-delivery)
 */
static SourceArena facts;
static AcquisitionSources origins;
static AcquisitionClosure closure;
static const AcquisitionVersionSource *origin;
static const DeliverySourceDefinition *definitions;
static ModulePackage prepared;
static bool refuse_test, declared_data;
static size_t expansions, admissions;
/** Retain actual independent frontend checks and failed diagnostics through plan teardown. */
typedef struct CheckedTest { struct CheckedTest *next; CompilerFrontend frontend; } CheckedTest;
static CheckedTest *checked_tests;
static const char *const parent_identity = "/test/fixture/v1.0.0/src/parent";
static const char *const child_identity = "/test/fixture/v1.0.0/src/parent/child";
/** Build two source units and their exact mirrored tests; an unrelated malformed
 * activated file is catalog metadata only and must never enter the active closure. */
static void prepare(bool promotion, bool open_test, bool injected, bool grounded) {
    assert(!(promotion && (injected || grounded)) && !(injected && grounded));
    refuse_test = false; declared_data = false; expansions = admissions = 0; checked_tests = NULL;
    fixture_package_prepare("generated/project", "fixture");
    directory("generated/project/packages/test/fixture/v1.0.0/src/parent+");
    directory("generated/project/packages/test/fixture/v1.0.0/tests/parent");
    directory("generated/project/packages/test/fixture/v1.0.0/tests/parent/child");
    file("generated/project/packages/test/fixture/v1.0.0/src/parent", promotion
        ? "@!\nnext&\n/test/fixture/v1.0.0/src/parent/child\n" : "unfinished source context\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/parent.ini", "");
    file("generated/project/packages/test/fixture/v1.0.0/src/parent+/child", injected
        ? "@<\nvalue\n.T\n@!\n" : promotion
        ? "@!\nback&\n/test/fixture/v1.0.0/src/parent\n" : "@!\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/parent+/child.extra", "retained companion\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/parent+/extra.data", "retained only after promotion\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/unused", "@!\ninvalid ! !\n");
    file("generated/project/packages/test/fixture/v1.0.0/tests/parent/check", "@!\ninstance\n/test/fixture/v1.0.0/src/parent\ninstance !\n");
    file("generated/project/packages/test/fixture/v1.0.0/tests/parent/helper", "@<\nvalue\n.T\n@!\n");
    if (!injected && !grounded) {
        int removed = unlink("generated/project/packages/test/fixture/v1.0.0/tests/parent/child/support/shape");
        assert(removed == 0 || errno == ENOENT);
        removed = unlink("generated/project/packages/test/fixture/v1.0.0/tests/parent/child/support/palette");
        assert(removed == 0 || errno == ENOENT);
        removed = rmdir("generated/project/packages/test/fixture/v1.0.0/tests/parent/child/support");
        assert(removed == 0 || errno == ENOENT);
    }
    if (injected || grounded) {
        directory("generated/project/packages/test/fixture/v1.0.0/tests/parent/child/support");
        if (injected) { file("generated/project/packages/test/fixture/v1.0.0/tests/parent/child/support/shape", "@!\n"); }
        if (grounded) { file("generated/project/packages/test/fixture/v1.0.0/tests/parent/child/support/palette",
            "@=\nvalue\n/test/fixture/v1.0.0/src/parent/child\n"); }
    }
    file("generated/project/packages/test/fixture/v1.0.0/tests/parent/child/check", grounded
        ? "@>\nresult\n.T\n ./support/palette/value\n@!\n" : injected
        ? "@!\nvalue\n./support/shape\ninstance\n/test/fixture/v1.0.0/src/parent/child\ninstance !\n < value\n" : open_test
        ? "@<\nvalue\n.T\n@!\n" : "@!\ninstance\n/test/fixture/v1.0.0/src/parent/child\ninstance !\n");
    file("generated/project/packages/test/fixture/v1.0.0/data.case", "explicit caller-selected experiment input\n");
    file("generated/project/packages/test/fixture/v1.0.0/README.md", "Reproduce these independent experiments.\n");
    setup_frontend(); source_arena_init(&facts, sources.allocator);
    acquisition_closure_init(&closure, sources.allocator);
    ModuleRoot root = {"generated/project", NULL, 0, MODULE_ROOT_PROJECT};
    ModuleStatus cataloged = module_catalog_build(&frontend.project.catalog, &root, 1); assert(cataloged == MODULE_OK);
    const char *required[] = {child_identity, "/test/fixture/v1.0.0/tests/parent/child/check",
        injected ? "/test/fixture/v1.0.0/tests/parent/child/support/shape"
            : grounded ? "/test/fixture/v1.0.0/tests/parent/child/support/palette" : parent_identity,
        "/test/fixture/v1.0.0/tests/parent/check", "/test/fixture/v1.0.0/tests/parent/helper"};
    bool checked = compiler_frontend_check_snapshot(&frontend, required, promotion ? 5 : (injected || grounded) ? 3 : 2);
    if (!checked) { fprintf(stderr, "Closure fixture failed normal checking: %s\n", frontend.error.message); }
    assert(checked);
    SourceStatus exported = delivery_source_export(&facts, &frontend.static_inventory, &definitions); assert(exported == SOURCE_OK);
    const ModuleSource *child = NULL;
    cataloged = module_catalog_find(&frontend.project.catalog, child_identity, &child); assert(cataloged == MODULE_OK);
    prepared = (ModulePackage){.logical = child->version->logical_path, .physical = child->version->physical_path, .local = true};
    acquisition_sources_init(&origins, sources.allocator, &prepared);
    AcquisitionSourceRequest request = {.local = &prepared}; AcquisitionSourcesIssue issue;
    AcquisitionSourcesStatus bound = acquisition_sources_bind(&origins, &request, &origin, &issue); assert(bound == ACQUISITION_SOURCES_OK);
}
/** Destroy plan views before independently checked programs, source facts and source contracts. */
static void finish(void) {
    acquisition_closure_destroy(&closure);
    for (CheckedTest *test = checked_tests; test; test = test->next) { compiler_frontend_destroy(&test->frontend); }
    acquisition_sources_destroy(&origins); source_arena_destroy(&facts); destroy_frontend();
}
/** Resolve one exact checked source definition; no filename convention or reparsing. */
static const ModuleSource *module_source(const char *identity) {
    const ModuleSource *source = NULL;
    ModuleStatus status = module_catalog_find(&frontend.project.catalog, identity, &source); assert(status == MODULE_OK);
    return source;
}
/** Capture a real module unit in the physical origin selected by the catalog. */
static SourceStatus capture_unit(const ModuleSource *source, AcquisitionRole role, const AcquisitionUnitEntry **output) {
    char *path = source_arena_copy(&facts, source_path(source->text));
    if (!path) { return SOURCE_OUT_OF_MEMORY; }
    char *slash = strrchr(path, '/'); assert(slash); *slash = 0;
    const char *failed = NULL;
    return acquisition_unit_capture(&facts, &sources, path, slash + 1, role, output, &failed);
}
/** Prepare complete source facts with the real capture and association components.
 * This fixture's topology has exactly parent and child, so its declared structural
 * child list is independently known rather than inferred from emitted output. */
static AcquisitionClosureStatus expand(void *context, const char *identity, AcquisitionExpansion *output, SourceIssue *issue) {
    (void)context; (void)issue; expansions++;
    const ModuleSource *source = module_source(identity);
    *output = (AcquisitionExpansion){.module = source, .source = origin, .definitions = definitions};
    SourceStatus captured = capture_unit(source, ACQUISITION_ACTIVE, &output->files);
    if (captured != SOURCE_OK) { return ACQUISITION_CLOSURE_PREPARATION_FAILED; }
    AcquisitionUnitEntry *configuration = source_arena_allocate(&facts, sizeof(*configuration)); assert(configuration);
    *configuration = (AcquisitionUnitEntry){.file = {.path = source_path(source->version->configuration),
        .content = source->version->configuration, .state = DELIVERY_FILE_PRESENT, .issue = PATH_FOUND}};
    output->configuration = configuration;
    AcquisitionUnitEntry *guidance = source_arena_allocate(&facts, sizeof(*guidance)); assert(guidance);
    *guidance = (AcquisitionUnitEntry){0};
    captured = delivery_file_capture(&facts, &sources, source->version->physical_path, "README.md", &guidance->file);
    assert(captured == SOURCE_OK && guidance->file.state == DELIVERY_FILE_PRESENT); output->guidance = guidance;
    if (declared_data && !strcmp(identity, child_identity)) {
        AcquisitionDeclaredInputs *inputs = source_arena_allocate(&facts, sizeof(*inputs)); assert(inputs);
        AcquisitionUnitEntry *data = source_arena_allocate(&facts, sizeof(*data)); assert(data);
        *data = (AcquisitionUnitEntry){0};
        captured = delivery_file_capture(&facts, &sources, source->version->physical_path, "data.case", &data->file);
        assert(captured == SOURCE_OK && data->file.state == DELIVERY_FILE_PRESENT);
        *inputs = (AcquisitionDeclaredInputs){.source = origin, .files = data}; output->declared_inputs = inputs;
    }
    if (!strcmp(identity, child_identity)) {
        AcquisitionContextFact *parent = source_arena_allocate(&facts, sizeof(*parent)); assert(parent);
        *parent = (AcquisitionContextFact){.identity = parent_identity};
        captured = capture_unit(module_source(parent_identity), ACQUISITION_CONTEXT, &parent->files); assert(captured == SOURCE_OK);
        output->contexts = parent;
    }
    if (!strcmp(identity, parent_identity)) {
        output->submodules = &child_identity; output->submodule_count = 1;
    }
    ModulePathShape shape;
    bool described = module_path_describe((const unsigned char *)identity, strlen(identity), &shape); assert(described);
    if (shape.area == MODULE_PATH_SRC) {
        AcquisitionTestDirectory *directory = source_arena_allocate(&facts, sizeof(*directory)); assert(directory);
        captured = acquisition_test_directory(&facts, source, directory); assert(captured == SOURCE_OK);
        output->test_directory = directory;
        char *path = source_arena_copy(&facts, directory->physical); assert(path);
        char *slash = strrchr(path, '/'); assert(slash); *slash = 0;
        const char *failed = NULL;
        captured = acquisition_directory_capture(&facts, &sources, path, slash + 1, &output->test_files, &failed);
        if (captured != SOURCE_OK) { return ACQUISITION_CLOSURE_PREPARATION_FAILED; }
    }
    return ACQUISITION_CLOSURE_OK;
}
/** Independently check each selected concrete program using the normal frontend;
 * helpers are never tried as roots and no native/CMake/test code executes. */
static AcquisitionClosureStatus admit(void *context, const DeliverySourceDefinition *definition, SourceIssue *issue) {
    (void)context;
    if (refuse_test) { issue->message = "Controlled independent test refusal"; return ACQUISITION_CLOSURE_PREPARATION_FAILED; }
    CheckedTest *test = source_arena_allocate(&facts, sizeof(*test)); assert(test);
    *test = (CheckedTest){.next = checked_tests}; checked_tests = test;
    compiler_frontend_init(&test->frontend, sources.allocator);
    ModuleRoot root = {context ? (const char *)context : "generated/project", NULL, 0, MODULE_ROOT_PROJECT};
    bool checked = compiler_frontend_check(&test->frontend, &root, 1, definition->identity);
    DeliveryEntryState state = delivery_source_entry_state(&test->frontend);
    if (!checked && state == DELIVERY_ENTRY_OPEN) { return ACQUISITION_CLOSURE_OPEN_TEST; }
    assert(checked && state == DELIVERY_ENTRY_EXECUTABLE); admissions++;
    return ACQUISITION_CLOSURE_OK;
}
/** Query captured membership independently from the controller's traversal order. */
static bool retained(const char *suffix) {
    const char *prefix = "generated/project/packages/test/fixture/v1.0.0/";
    for (const AcquisitionMember *member = closure.members.first; member; member = member->next) {
        if (!strncmp(member->path, prefix, strlen(prefix)) && !strcmp(member->path + strlen(prefix), suffix)) {
            assert(member->causes && member->causes->source == origin); return true;
        }
    }
    return false;
}
