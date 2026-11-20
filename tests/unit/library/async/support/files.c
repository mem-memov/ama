/** @file
 * @brief Load maintained native/Ama integration sources without treating generated code as an oracle.
 */
/** Copy a bounded maintained input as immutable test-owned bytes. */
static void input_text(const char *path, char bytes[32768]) {
    FILE *input = fopen(path, "rb");
    assert(input != NULL);
    size_t count = fread(bytes, 1, 32767, input);
    assert(!ferror(input) && feof(input));
    bytes[count] = '\0';
    int closed = fclose(input);
    assert(closed == 0);
}
/** Install the actual supported root and its typed native companion set before frontend checking. */
static void install_project(void) {
    const char *names[] = {"main", "invalid", "submit", "submit.h", "submit.c", "submit.abi"};
    for (size_t index = 0; index < sizeof(names) / sizeof(*names); index++) {
        char input[256], output[256], bytes[32768];
        int length = snprintf(input, sizeof(input), TEST_ASYNC_HOST "/project/packages/test/fixture/v1.0.0/src/%s", names[index]);
        assert(length > 0 && (size_t)length < sizeof(input));
        input_text(input, bytes);
        length = snprintf(output, sizeof(output), "generated/project/packages/test/fixture/v1.0.0/src/%s", names[index]);
        assert(length > 0 && (size_t)length < sizeof(output));
        file(output, bytes);
    }
}
