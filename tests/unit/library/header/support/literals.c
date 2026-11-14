/** @file
 * @brief Preserve arbitrary declared UTF-8 metadata as safe C literals and deterministic headers.
 */
/** Quotes, backslashes and trigraph-sensitive sequences remain exact bytes in a standalone C consumer. */
static void it_preserves_metadata_octets_without_injecting_c_source(void) {
    prepare_project(); setup_test();
    const char *name = "пример\"?" "?/\\suffix*/";
    LibraryBuild build; component(&build, name);
    header(&build, "generated/literal.h");
    char macro[71];
    const char *p = build.identity->internal_name;
    for (size_t i = 0; i <= strlen(p); i++) { macro[i] = p[i] >= 'a' && p[i] <= 'z' ? (char)(p[i] - 'a' + 'A') : p[i]; }
    FILE *source = fopen("generated/literal.c", "wb"); assert(source != NULL);
    int written = fprintf(source, "#include \"literal.h\"\n#include <assert.h>\nint main(void){const uint8_t expected[]={");
    assert(written > 0);
    for (size_t i = 0; i < strlen(name); i++) {
        written = fprintf(source, "%s%u", i == 0 ? "" : ",", (unsigned)(unsigned char)name[i]);
        assert(written > 0);
    }
    written = fprintf(source, "};const char *actual=%s_COMPONENT_ID;assert(sizeof(%s_COMPONENT_ID)==sizeof(expected)+1);for(size_t i=0;i<sizeof(expected);i++){assert((uint8_t)actual[i]==expected[i]);}return 0;}\n", macro,macro);
    assert(written > 0);
    int closed = fclose(source); assert(closed == 0);
    char diagnostic[32768];
    bool compiled = compile("generated/literal.c", NULL, "generated/literal", true, NULL, diagnostic, sizeof(diagnostic));
    if (!compiled) { fprintf(stderr, "%s", diagnostic); }
    assert(compiled);
    execute("generated/literal");
    FILE *a = tmpfile(), *b = tmpfile(); assert(a != NULL && b != NULL);
    bool emitted = library_header_write(&build, a) && library_header_write(&build, b); assert(emitted);
    int seek_a = fseek(a, 0, SEEK_SET), seek_b = fseek(b, 0, SEEK_SET); assert(seek_a == 0 && seek_b == 0);
    for (;;) {
        unsigned char left[512], right[512];
        size_t x = fread(left, 1, sizeof(left), a), y = fread(right, 1, sizeof(right), b);
        assert(!ferror(a) && !ferror(b) && x == y && memcmp(left, right, x) == 0);
        if (x == 0) { break; }
    }
    int close_a = fclose(a), close_b = fclose(b); assert(close_a == 0 && close_b == 0);
    library_build_destroy(&build); destroy_test();
}
