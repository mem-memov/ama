/** @file
 * @brief Verify exact public symbol inventories independently of native symbol spelling.
 */
/** Two exports share nominal types but selected operations remain independently enumerated. */
static void it_enumerates_only_declared_public_component_definitions(void) {
    setup_test(); prepare_project();
    LibraryBuild build; component(&build, "inventory.component");
    FILE *output = tmpfile(); assert(output != NULL);
    bool written = library_symbols_write(&build, output); assert(written);
    int seek = fseek(output, 0, SEEK_SET); assert(seek == 0);
    char line[256]; size_t count = 0;
    while (fgets(line, sizeof(line), output) != NULL) {
        size_t length = strlen(line); assert(length != 0 && line[length - 1] == '\n');
        assert(strstr(line, "_implementation\n") != NULL);
        assert(strncmp(line, "ama_r_", 6) == 0);
        if (count == 0) {
            char expected[256];
            int size = snprintf(expected, sizeof(expected), "%s_abi_implementation\n", build.identity->internal_name);
            assert(size > 0 && (size_t)size < sizeof(expected));
            assert(strcmp(line, expected) == 0);
        }
        count++;
    }
    /* Ten lifecycle symbols, five schemas with three operations each,
     * one full export with four symbols, and one read-only export with two. */
    assert(!ferror(output) && count == 10 + 5 * 3 + 4 + 2);
    int closed = fclose(output); assert(closed == 0);
    library_build_destroy(&build); destroy_test();
}
