/** @file
 * @brief Verify the real separate Makefile using private build/output overrides.
 * @see [T233 contract](../../../../../docs/implementation/project-tool.md)
 */
#define _XOPEN_SOURCE 700
#include "../common.c"
#include "../../../../../tools/project/process/process.h"
/** Execute Make with exact argument-vector assignments, keeping all outputs private. */
static int build(const char *binary, const char *objects, const char *override, FILE *output, FILE *errors) {
    char executable[640], directory[640];
    int length = snprintf(executable, sizeof(executable), "BINARY=%s", binary); assert(length > 0 && (size_t)length < sizeof(executable));
    length = snprintf(directory, sizeof(directory), "CONFIGURATION_DIRECTORY=%s", objects); assert(length > 0 && (size_t)length < sizeof(directory));
    char *arguments[] = {"make", "-C", PROJECT_MAKE_DIRECTORY, executable, directory, (char *)override, NULL};
    if (!override) { arguments[5] = NULL; }
    ProjectProcessResult result;
    bool observed = project_process(arguments, NULL, output, errors, &result);
    assert(observed && !result.error && !result.signal);
    return result.exit_code;
}
/** Exercise real command/link invalidation and successful recovery in a private build tree. */
static void it_builds_independently_and_tracks_flags_and_exact_link_membership(void) {
    setup();
    char *physical = realpath(root, NULL); assert(physical);
    char binary[512], objects[512];
    (void)snprintf(binary, sizeof(binary), "%s/ama-project", physical);
    (void)snprintf(objects, sizeof(objects), "%s/objects", physical);
    FILE *output = tmpfile(), *errors = tmpfile(); assert(output && errors);
    int status = build(binary, objects, NULL, output, errors); assert(status == 0);
    struct stat before, after; int read = stat(binary, &before); assert(!read);
    status = build(binary, objects, NULL, output, errors); assert(status == 0);
    read = stat(binary, &after); assert(!read);
    assert(before.st_mtim.tv_sec == after.st_mtim.tv_sec && before.st_mtim.tv_nsec == after.st_mtim.tv_nsec);
    /* Existing objects must not hide removed link inputs. */
    status = build(binary, objects, "PROJECT_TOOL_SHARED_SOURCES=", output, errors); assert(status != 0);
    status = build(binary, objects, NULL, output, errors); assert(status == 0);
    /* A changed command must actually reach the compiler, including refusal. */
    status = build(binary, objects, "C_COMPILER_FLAGS=-fama-invalid-test-flag", output, errors); assert(status != 0);
    status = build(binary, objects, NULL, output, errors); assert(status == 0);
    char *arguments[] = {binary, "init", destination, NULL};
    ProjectProcessResult result;
    bool observed = project_process(arguments, NULL, output, errors, &result);
    assert(observed && !result.error && !result.exit_code && !result.signal);
    char path[256]; (void)snprintf(path, sizeof(path), "%s/AGENTS.md", destination);
    FILE *guidance = fopen(path, "rb"); assert(guidance);
    char text[2048]; size_t bytes = fread(text, 1, sizeof(text) - 1, guidance); text[bytes] = 0;
    assert(strstr(text, "# Guidance for Ama contributors") && members(root) == 3);
    int closed = fclose(guidance); assert(!closed);
    closed = fclose(output); assert(!closed); closed = fclose(errors); assert(!closed);
    free(physical); destroy();
}
/** Execute every named assertion case explicitly in the intended order. */
void run(void) { it_builds_independently_and_tracks_flags_and_exact_link_membership(); }
/** Adapt the harness process to its explicit assertion cases and child controls. */
int main(void) { run(); return 0; }
