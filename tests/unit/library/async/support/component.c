/** @file
 * @brief Package the actual asynchronous Ama root with its declared external native dependency and immutable wire schemas.
 */
/** Build one actual component, with native callbacks bound through generated SDK declarations rather than test stubs. */
static void build_component(LibraryBuild *build, const char *backend, const char *include) {
    char records[8192];
    int length = snprintf(records, sizeof(records),
        "schema | text | text | //text\nschema | flag | condition | //std/condition\n"
        "export | run | /test/fixture/v1.0.0/src/main | entry,resume\ninput | run | immediate | flag\ninput | run | cancel requested | flag\noutput | run | message | text\n"
        "export | invalid | /test/fixture/v1.0.0/src/invalid | entry\n"
        "c-flag | -I%s\nc-flag | -DNDEBUG\nlink-file | %s\n"
#ifdef TEST_THREAD_SANITIZER
        "c-flag | -fsanitize=thread\nc-flag | -fno-omit-frame-pointer\nlink-flag | -fsanitize=thread\n"
#elif defined(TEST_SANITIZERS)
        "c-flag | -fsanitize=address,undefined\nc-flag | -fno-omit-frame-pointer\nlink-flag | -fsanitize=address,undefined\n"
#endif
        , include, backend);
    assert(length > 0 && (size_t)length < sizeof(records));
    const SourceText *source = configuration(records);
    library_build_init(build, sources.allocator);
    bool valid = library_configuration_parse(build, source);
    assert(valid);
    ModuleRoot roots[] = {{.path = "generated/project", .kind = MODULE_ROOT_PROJECT}, {.path = TEST_STANDARD_ROOT, .kind = MODULE_ROOT_STANDARD}};
    valid = library_build_resolve(build, roots, 2);
    if (!valid) { fprintf(stderr, "Library model: %s\n", build->error.message); }
    assert(valid);
    NativeScaffold assembly;
    native_scaffold_init(&assembly, &build->frontend, sources.allocator);
    bool built = project_codegen_build(&assembly, (ScaffoldOptions){.directory = "generated/component", .sdk = TEST_SDK});
    assert(built);
    bool published = native_scaffold_publish(&assembly);
    assert(published);
    FILE *output = fopen("generated/component/public.h", "wb");
    assert(output != NULL);
    bool written = library_header_write(build, output);
    int closed = fclose(output);
    assert(written && closed == 0);
    output = fopen("generated/component/component.c", "wb");
    assert(output != NULL);
    written = library_assembly_write(build, &assembly, "public.h", "project.c", output);
    closed = fclose(output);
    assert(written && closed == 0);
    const LibraryPackageOptions options = {.assembly = &assembly, .compiler = TEST_C_COMPILER, .archiver = "ar", .object_copy = "objcopy",
        .source = "generated/component/component.c", .sdk = TEST_SDK, .includes = "generated/component",
        .raw_object = "generated/component/raw.o", .isolated_object = "generated/component/isolated.o",
        .symbols = "generated/component/symbols.txt", .archive = "generated/component/library.a", .shared = "generated/component/library.so"};
    built = library_package_build(build, &options, stderr);
    assert(built);
    native_scaffold_destroy(&assembly);
}
