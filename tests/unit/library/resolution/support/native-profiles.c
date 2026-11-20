/** @file
 * @brief Require explicit native capabilities and exact failure ownership for wire profiles.
 */
/** Install one test-owned native interface; C execution would deliberately fail compilation. */
static void profile_native(const char *name, const char *interface, const char *abi) {
    const char *suffixes[] = {"", ".h", ".c", ".abi"};
    for (size_t index = 0; index < sizeof(suffixes) / sizeof(*suffixes); index++) {
        char path[128];
        int length = snprintf(path, sizeof(path), "generated/standard/%s%s", name, suffixes[index]);
        assert(length > 0 && (size_t)length < sizeof(path));
        file(path, index == 0 ? interface : index == 3 ? abi : "#error Native C must not run during library checking\n");
    }
}
/** Private payloads are never accepted just because their logical basename looks standard. */
static void it_requires_native_profile_capabilities_and_failure_shape(void) {
    prepare_project();
    directory("generated/standard");
    file("generated/project/packages/test/fixture/v1.0.0/src/empty", "@!\n");
    const char *failure_interfaces[] = {
        "@>\ndescription\n//text\n@C\n", "@>\nreason\n//text\n@C\n",
        "@>\ndescription&\n//text\n@C\n", "@>\ndescription[]\n//text\n@C\n"
    };
    for (size_t shape = 0; shape < 8; shape++) {
        profile_native("text", shape == 6 ? "@!\n" : "@C\n", shape == 0
            ? "+ initialize\n- destroy\n| literal\n? condition\n"
            : shape == 1 ? "+ initialize\n- destroy\n| literal\n= clone\n"
            : "+ initialize\n- destroy\n| literal\n= clone\n? condition\n");
        profile_native("failure", failure_interfaces[shape >= 2 && shape <= 4 ? shape - 1 : 0], shape == 5
            ? "+ initialize\n- destroy\n= clone\n" : "+ initialize\n- destroy\n= clone\n? condition\n");
        setup_test();
        const SourceText *source = configuration("schema | outcome | failure | //failure\nexport | empty | /test/fixture/v1.0.0/src/empty | entry\n");
        LibraryBuild build;
        library_build_init(&build, sources.allocator);
        bool parsed = library_configuration_parse(&build, source);
        assert(parsed);
        ModuleRoot roots[] = {{.path = "generated/project", .kind = MODULE_ROOT_PROJECT},
            {.path = "generated/standard", .kind = MODULE_ROOT_STANDARD}};
        bool valid = library_build_resolve(&build, roots, 2);
        assert(valid == (shape == 7));
        if (!valid) { assert(diagnostic_valid(&build.error) && !build.valid); }
        library_build_destroy(&build);
        destroy_test();
    }
}
