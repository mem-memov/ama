/** @file
 * @brief Reject wrong ABI editions, cross-component types and leaked private-runtime expectations.
 */
/** Independent public namespaces reject accidental foreign handles at C compilation. */
static void it_rejects_incompatible_headers_and_private_type_dependencies(void) {
    prepare_project();
    setup_test();
    LibraryBuild first, second;
    component(&first, "example.first"); component(&second, "example.second");
    header(&first, "generated/first.h"); header(&second, "generated/second.h");
    char source[4096], diagnostic[32768], macro[71];
    const char *p = first.identity->internal_name, *q = second.identity->internal_name;
    for (size_t i = 0; i <= strlen(p); i++) { macro[i] = p[i] >= 'a' && p[i] <= 'z' ? (char)(p[i] - 'a' + 'A') : p[i]; }
    int length = snprintf(source, sizeof(source), "#define %s_REQUIRED_ABI 2\n#include \"first.h\"\n", macro);
    assert(length > 0 && (size_t)length < sizeof(source));
    file("generated/negative.c", source);
    bool compiled = compile("generated/negative.c", NULL, "generated/negative.o", false, NULL, diagnostic, sizeof(diagnostic));
    assert(!compiled && strstr(diagnostic, "Unsupported requested Ama C ABI edition") != NULL);
    length = snprintf(source, sizeof(source), "#include \"first.h\"\n#include \"second.h\"\nvoid wrong(%s_Context first) { %s_Context second=first;(void)second;}\n", p,q);
    assert(length > 0 && (size_t)length < sizeof(source));
    file("generated/negative.c", source);
    compiled = compile("generated/negative.c", NULL, "generated/negative.o", false, NULL, diagnostic, sizeof(diagnostic));
    assert(!compiled);
    file("generated/negative.c", "#include \"first.h\"\nAmaStatus leaked;\n");
    compiled = compile("generated/negative.c", NULL, "generated/negative.o", false, NULL, diagnostic, sizeof(diagnostic));
    assert(!compiled && strstr(diagnostic, "AmaStatus") != NULL);
    file("generated/negative.c", "#include \"first.h\"\n");
    compiled = compile("generated/negative.c", NULL, "generated/negative.o", false, "-mabi=ms", diagnostic, sizeof(diagnostic));
    assert(!compiled && strstr(diagnostic, "Unsupported default C calling convention") != NULL);
    const char *e = first.exports->next->identity->internal_name;
    length = snprintf(source, sizeof(source), "#include \"first.h\"\nvoid unavailable(void) { (void)%s_entry; }\n", e);
    assert(length > 0 && (size_t)length < sizeof(source));
    file("generated/negative.c", source);
    compiled = compile("generated/negative.c", NULL, "generated/negative.o", false, NULL, diagnostic, sizeof(diagnostic));
    assert(!compiled);
    library_build_destroy(&second); library_build_destroy(&first); destroy_test();
}
/** A resolved metadata target outside the supported header profile emits no partial C source. */
static void it_rejects_unsupported_targets_before_header_publication(void) {
    prepare_project(); setup_test();
    const SourceText *original = configuration(records);
    const char *bytes = (const char *)source_bytes(original);
    const char *old = strstr(bytes, "x86_64-linux-gnu"); assert(old != NULL);
    char text[8192];
    int length = snprintf(text, sizeof(text), "%.*sunsupported-target%s", (int)(old - bytes), bytes, old + strlen("x86_64-linux-gnu"));
    assert(length > 0 && (size_t)length < sizeof(text));
    const SourceText *source = NULL;
    SourceStatus added = source_manager_add(&sources, "unsupported.exports", (const unsigned char *)text, (size_t)length, &source);
    assert(added == SOURCE_OK);
    LibraryBuild build; library_build_init(&build, sources.allocator);
    bool parsed = library_configuration_parse(&build, source); assert(parsed);
    ModuleRoot roots[] = {{.path="generated/project",.kind=MODULE_ROOT_PROJECT}, {.path=TEST_STANDARD_ROOT,.kind=MODULE_ROOT_STANDARD}};
    bool valid = library_build_resolve(&build, roots, 2); assert(valid);
    Diagnostic diagnostic = {0};
    bool supported = library_header_target(&build, &diagnostic);
    assert(!supported && diagnostic.code == DIAGNOSTIC_UNSUPPORTED_FEATURE && diagnostic_valid(&diagnostic));
    SourcePosition position;
    bool located = source_position(source, diagnostic.primary.span.start, &position);
    assert(located && position.line == 4 && position.column == 1);
    FILE *output = tmpfile(); assert(output != NULL);
    bool emitted = library_header_write(&build, output); assert(!emitted);
    long size = ftell(output); assert(size == 0);
    emitted = library_abi_write(&build, output); assert(!emitted);
    size = ftell(output); assert(size == 0);
    int closed = fclose(output); assert(closed == 0);
    library_build_destroy(&build); destroy_test();
}
