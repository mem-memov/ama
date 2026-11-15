/** @file
 * @brief Compile a separately resident native dependency with direct checked argument vectors.
 */
/** Resolve explicit paths so the supported ELF dependency remains available across component reloads. */
static void build_backend(char backend[4096], char include[4096]) {
    char cwd[3072];
    char *directory_path = getcwd(cwd, sizeof(cwd));
    assert(directory_path != NULL);
    int length = snprintf(backend, 4096, "%s/generated/backend.so", cwd);
    assert(length > 0 && length < 4096);
    length = snprintf(include, 4096, "%s/backend", TEST_ASYNC_HOST);
    assert(length > 0 && length < 4096);
    char *arguments[32] = {TEST_C_COMPILER, "-std=c11", "-D_POSIX_C_SOURCE=200809L", "-Wall", "-Wextra", "-Werror", "-pedantic",
        "-O1", "-g", "-UNDEBUG", "-fPIC", "-shared", "-pthread", "-I", TEST_SDK};
    size_t count = 15;
#ifdef TEST_THREAD_SANITIZER
    arguments[count++] = "-fsanitize=thread";
    arguments[count++] = "-fno-omit-frame-pointer";
#elif defined(TEST_SANITIZERS)
    arguments[count++] = "-fsanitize=address,undefined";
    arguments[count++] = "-fno-omit-frame-pointer";
#endif
    arguments[count++] = TEST_ASYNC_HOST "/backend/backend.c";
    arguments[count++] = "-o";
    arguments[count++] = backend;
    arguments[count] = NULL;
    bool compiled = library_package_process(arguments, stderr);
    assert(compiled);
}
