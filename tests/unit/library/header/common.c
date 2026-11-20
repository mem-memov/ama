/** @file
 * @brief Generate checked component headers and compile isolated consumers through real C11.
 */
#ifndef AMA_PUBLIC_HEADER_TEST_COMMON
#define AMA_PUBLIC_HEADER_TEST_COMMON
#include <sys/wait.h>
#include <unistd.h>
#include "../common.c"
/** Explicit models exercise every public wire profile and both full/read-only exports. */
static const char records[] = "schema | text | text | //text\nschema | number | integer | //integer\n"
    "schema | enabled | condition | //std/condition\nschema | failure | failure | //failure\nschema | texts | list | text\n"
    "export | run | /test/fixture/v1.0.0/src/main | entry,resume,read\ninput | run | message text | text\noutput | run | failure | failure\nexport | observer | /test/fixture/v1.0.0/src/other | read\n";
/** Create source fixtures without generating or inspecting any private native header. */
static void prepare_project(void) {
    directory("generated/project");
    fixture_package_prepare("generated/project", "fixture");
    file("generated/project/packages/test/fixture/v1.0.0/src/main", "@<\nmessage text\n//text\n@>\nfailure\n//failure\n@!\n");
    file("generated/project/packages/test/fixture/v1.0.0/src/other", "@!\n");
}
/** Build an explicitly named component with the same Ama module identities as its peer. */
static void component(LibraryBuild *build, const char *name) {
    const SourceText *original = configuration(records);
    const char *bytes = (const char *)source_bytes(original);
    const char *old = strstr(bytes, "example.component");
    assert(old != NULL);
    char text[8192];
    int length = snprintf(text, sizeof(text), "%.*s%s%s", (int)(old - bytes), bytes, name, old + strlen("example.component"));
    assert(length > 0 && (size_t)length < sizeof(text));
    const SourceText *source = NULL;
    SourceStatus added = source_manager_add(&sources, "component.exports", (const unsigned char *)text, (size_t)length, &source);
    assert(added == SOURCE_OK);
    library_build_init(build, sources.allocator);
    bool parsed = library_configuration_parse(build, source);
    assert(parsed);
    ModuleRoot roots[] = {{.path = "generated/project", .kind = MODULE_ROOT_PROJECT},
        {.path = TEST_STANDARD_ROOT, .kind = MODULE_ROOT_STANDARD}};
    bool valid = library_build_resolve(build, roots, 2);
    if (!valid) { fprintf(stderr, "%s\n", build->error.message); }
    assert(valid);
}
/** Publish only a consumer header to test-owned generated storage; assert model immutability. */
static void header(const LibraryBuild *build, const char *path) {
    FILE *output = fopen(path, "wb");
    assert(output != NULL);
    size_t before = allocation_count;
    bool written = library_header_write(build, output);
    int closed = fclose(output);
    assert(written && closed == 0 && allocation_count == before);
}
/** Use one real C process, with no shell or compiler SDK include path. */
static bool compile(const char *source, const char *peer, const char *output, bool link, const char *extra, char *diagnostic, size_t capacity) {
    FILE *errors = tmpfile();
    assert(errors != NULL);
    pid_t child = fork();
    assert(child >= 0);
    if (child == 0) {
        if (dup2(fileno(errors), STDERR_FILENO) < 0) { _exit(126); }
        char *arguments[24] = {TEST_C_COMPILER, "-std=c11", "-Wall", "-Wextra", "-Werror", "-pedantic-errors"};
        size_t count = 6;
#ifdef TEST_SANITIZERS
        if (link) {
            arguments[count++] = "-fsanitize=address,undefined";
            arguments[count++] = "-fno-omit-frame-pointer";
            arguments[count++] = "-fno-pie";
            arguments[count++] = "-no-pie";
        }
#endif
        if (extra != NULL) { arguments[count++] = (char *)extra; }
        if (!link) { arguments[count++] = "-c"; }
        arguments[count++] = (char *)source;
        if (peer != NULL) { arguments[count++] = (char *)peer; }
        arguments[count++] = "-o"; arguments[count++] = (char *)output;
        arguments[count] = NULL;
        execvp(TEST_C_COMPILER, arguments);
        _exit(127);
    }
    int status;
    pid_t waited;
    do { waited = waitpid(child, &status, 0); } while (waited < 0 && errno == EINTR);
    assert(waited == child && WIFEXITED(status));
    int seek = fseek(errors, 0, SEEK_SET);
    assert(seek == 0);
    size_t length = fread(diagnostic, 1, capacity - 1, errors);
    assert(!ferror(errors) && length < capacity - 1);
    diagnostic[length] = 0;
    int closed = fclose(errors);
    assert(closed == 0);
    return WEXITSTATUS(status) == 0;
}
/** Execute the independent C ABI-gate probe and require exact successful exit. */
static void execute(const char *program) {
    pid_t child = fork();
    assert(child >= 0);
    if (child == 0) { execl(program, program, (char *)NULL); _exit(127); }
    int status;
    pid_t waited;
    do { waited = waitpid(child, &status, 0); } while (waited < 0 && errno == EINTR);
    assert(waited == child && WIFEXITED(status) && WEXITSTATUS(status) == 0);
}
#endif
