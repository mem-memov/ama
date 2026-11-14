/** @file
 * @brief Inject every allocation failure across complete library resolution.
 */
/** Every failed allocation of a complete typed library check unwinds all independent owners. */
static void it_unwinds_every_library_resolution_allocation(void) {
    prepare_project();
    setup_test();
    LibraryBuild build;
    bool valid = resolve(&build, records);
    assert(valid);
    size_t count = allocation_count;
    library_build_destroy(&build);
    destroy_test();
    for (size_t failure = 1; failure <= count; failure++) {
        setup_test();
        /* Source construction itself is the fixture's responsibility, so inject
         * only after the borrowed configuration source is safely owned below.
         */
        const SourceText *source = configuration(records);
        size_t prefix = allocation_count;
        if (failure <= prefix) { destroy_test(); continue; }
        failure_at = failure;
        library_build_init(&build, sources.allocator);
        bool parsed = library_configuration_parse(&build, source);
        if (parsed) {
            ModuleRoot roots[] = {{.path = "generated/project", .kind = MODULE_ROOT_PROJECT},
                {.path = TEST_STANDARD_ROOT, .kind = MODULE_ROOT_STANDARD}};
            valid = library_build_resolve(&build, roots, 2);
        } else { valid = false; }
        assert(!valid && !build.valid && build.error.code == DIAGNOSTIC_OUT_OF_MEMORY);
        library_build_destroy(&build);
        destroy_test();
    }
}
