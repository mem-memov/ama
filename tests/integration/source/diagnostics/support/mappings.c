/**
 * @file
 * @brief Compile exact filename-byte and line-coordinate round trips across the ASCII escape domain.
 */

/** Every supported non-NUL ASCII byte and control-following digit survives C preprocessing; coordinates include CRLF and EOF. */
static void it_compiles_every_ascii_filename_byte_and_physical_line_boundary(void) {
    setup_test();
    char path[256];
    size_t used = 0;
    for (unsigned byte = 1; byte <= 127; byte++) {
        if (byte == 13) {
            continue;
        }
        path[used++] = (char)byte;
        if (byte < 32 || byte == 127) {
            path[used++] = '7';
        }
    }
    /* Both potential trigraph prefixes and original non-ASCII bytes remain data. */
    const char suffix[] = "?" "?=" "?" "?/" " 商品";
    memcpy(path + used, suffix, sizeof(suffix));
    const unsigned char bytes[] = {'a', '\r', '\n', 'b', '\n', 'c'};
    const size_t lines[] = {1, 1, 1, 2, 2, 3, 3};
    const SourceText *source = NULL;
    SourceStatus added = source_manager_add(&catalog.sources, path, bytes, sizeof(bytes), &source);
    assert(added == SOURCE_OK);
    FILE *output = fopen("generated/all-mappings.c", "wb");
    assert(output != NULL);
    int written = fputs("#include <string.h>\nstatic const unsigned char expected[] = {", output);
    assert(written != EOF);
    for (const unsigned char *byte = (const unsigned char *)path; *byte != 0; byte++) {
        written = fprintf(output, "%u,", (unsigned int)*byte);
        assert(written >= 0);
    }
    written = fputs("0};\n", output);
    assert(written != EOF);
    for (size_t offset = 0; offset <= sizeof(bytes); offset++) {
        SourceStatus mapped = source_write_line_directive(output, source, offset);
        assert(mapped == SOURCE_OK);
        written = fprintf(output,
            "static int probe%zu(void) { return __LINE__ != %zu || strcmp(__FILE__, (const char *)expected) != 0; }\n",
            offset, lines[offset]);
        assert(written >= 0);
    }
    written = fputs("int main(void) { return ", output);
    assert(written != EOF);
    for (size_t offset = 0; offset <= sizeof(bytes); offset++) {
        written = fprintf(output, "%sprobe%zu()", offset == 0 ? "" : " || ", offset);
        assert(written >= 0);
    }
    written = fputs("; }\n", output);
    assert(written != EOF);
    int closed = fclose(output);
    assert(closed == 0);
    char *compiler[] = {TEST_REAL_COMPILER, "-std=c11", "-pedantic-errors", "-Wall", "-Wextra", "-Werror",
        "-trigraphs", "-finput-charset=UTF-8", "-fexec-charset=UTF-8",
        "generated/all-mappings.c", "-o", "generated/all-mappings", NULL};
    run_process(compiler);
    char *program[] = {"./generated/all-mappings", NULL};
    run_process(program);
    destroy_test();
}
