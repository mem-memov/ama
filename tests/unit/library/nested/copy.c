/** @file
 * @brief Reject both value and continuation copies of an exclusively owned child resource.
 */
/** Absence of native '=' propagates through ordinary ownership before any C code runs. */
static void it_rejects_ordinary_and_continuation_copy_of_the_child_owner(void) {
    setup_test(); directory("generated/copy-root");
    const char *programs[]={"@!\noriginal\n.adapter\ncopy\n original\n",
        "@!\noriginal\n.adapter\ncopy\n % original\n"};
    for(size_t i=0;i<sizeof(programs)/sizeof(*programs);i++) {
        file("generated/copy-root/main",programs[i]);
        CompilerFrontend frontend; compiler_frontend_init(&frontend,sources.allocator);
        ModuleRoot roots[]={{.path="generated/copy-root",.kind=MODULE_ROOT_PROJECT},
            {.path="project",.kind=MODULE_ROOT_PROJECT},{.path=TEST_STANDARD_ROOT,.kind=MODULE_ROOT_STANDARD}};
        bool valid=compiler_frontend_check(&frontend,roots,3,".main");
        assert(!valid && frontend.error.code==DIAGNOSTIC_TYPE_MISMATCH);
        assert(strstr(frontend.error.message,"independent copying")!=NULL);
        assert(frontend.error.cause!=NULL && strstr(frontend.error.cause,"no declared =")!=NULL);
        compiler_frontend_destroy(&frontend);
    }
    destroy_test();
}
/** The public C transport cannot export an Ama reference as if it were an independent text value. */
static void it_rejects_reference_transport_across_the_component_boundary(void) {
    setup_test(); directory("generated/copy-root");
    file("generated/copy-root/reference","@>\ninner&\n..text\n@!\n");
    const SourceText *source=configuration("schema | text | text | ..text\nexport | run | .reference | read\noutput | run | inner | text\n");
    LibraryBuild build; library_build_init(&build,sources.allocator);
    bool parsed=library_configuration_parse(&build,source); assert(parsed);
    ModuleRoot roots[]={{.path="generated/copy-root",.kind=MODULE_ROOT_PROJECT},
        {.path=TEST_STANDARD_ROOT,.kind=MODULE_ROOT_STANDARD}};
    bool valid=library_build_resolve(&build,roots,2);
    assert(!valid && build.error.code==DIAGNOSTIC_TYPE_MISMATCH);
    assert(strstr(build.error.message,"owned transport schema")!=NULL);
    library_build_destroy(&build); destroy_test();
}
