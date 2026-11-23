/** @file
 * @brief Verify physical endpoint observations against POSIX oracles without following aliases or erasing missing traversal.
 * @see [T224 contract](../../../../../docs/implementation/source-operations.md)
 */
#define _XOPEN_SOURCE 700
#include <errno.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include "../../common.c"
#include "source/path/physical/physical.h"
static SourceArena arena;
/** Own bounded known fixture directories, preserving unexpected errors. */
static void directory(const char *path) { assert(mkdir(path, 0700) == 0 || errno == EEXIST); }
/** Prepare a regular file without reading it in the resolver. */
static void file(const char *path) { FILE *stream = fopen(path, "wb"); assert(stream && fclose(stream) == 0); }
/** Fixed byte prefixes and aliases exercise physical naming separately from Ama identifiers. */
static void fixtures(void) {
    directory("generated/tree"); directory("generated/tree/directory"); directory("generated/tree/\xff zone");
    file("generated/tree/regular"); file("generated/tree/\xff zone/name");
    assert(symlink("directory", "generated/tree/link") == 0 || errno == EEXIST);
    assert(symlink("absent", "generated/tree/broken") == 0 || errno == EEXIST);
    assert(mkfifo("generated/tree/pipe", 0600) == 0 || errno == EEXIST);
}
/** Begin counted path-only ownership. */
static void prepare(void) { setup_test(); source_arena_init(&arena, sources.allocator); }
/** Retire failed and successful scratch before checking all counted allocations. */
static void finish(void) { source_arena_destroy(&arena); destroy_test(); }
/** Existing exact positive paths agree with an independent POSIX canonical oracle. */
static void it_matches_existing_directory_file_and_raw_byte_paths_against_posix(void) {
    prepare();
    const char *paths[] = {"generated//tree/directory/../regular", "generated/tree/./directory", "generated/tree/\xff zone/name", "/./../"};
    for (size_t index = 0; index < 4; index++) {
        char *expected = realpath(paths[index], NULL); assert(expected);
        SourcePhysicalPath result; SourcePhysicalIssue issue;
        assert(source_physical_resolve(&arena, paths[index], SOURCE_PHYSICAL_ENDPOINT, false, &result, &issue) == SOURCE_PHYSICAL_OK);
        assert(result.present && !strcmp(result.path, expected) && issue.status == SOURCE_PHYSICAL_OK);
        struct stat metadata; assert(lstat(expected, &metadata) == 0);
        assert(result.metadata.st_dev == metadata.st_dev && result.metadata.st_ino == metadata.st_ino);
        free(expected);
    }
    char input[] = "generated/tree/regular";
    SourcePhysicalPath result; SourcePhysicalIssue issue;
    assert(source_physical_resolve(&arena, input, SOURCE_PHYSICAL_ENDPOINT, false, &result, &issue) == SOURCE_PHYSICAL_OK);
    input[0] = '!'; assert(strstr(result.path, "/generated/tree/regular")); finish();
}
/** Missing final is explicit and preserves directory intent without creating anything. */
static void it_distinguishes_missing_final_destinations_from_missing_intermediate_traversal(void) {
    prepare(); SourcePhysicalPath result; SourcePhysicalIssue issue;
    assert(source_physical_resolve(&arena, "generated/tree/new/", SOURCE_PHYSICAL_ENDPOINT, true, &result, &issue) == SOURCE_PHYSICAL_OK);
    assert(!result.present && result.directory_required && !result.metadata.st_ino);
    struct stat metadata; assert(lstat(result.path, &metadata) == -1 && errno == ENOENT);
    result = (SourcePhysicalPath){.path = "unchanged", .present = true};
    assert(source_physical_resolve(&arena, "generated/tree/new", SOURCE_PHYSICAL_ENDPOINT, false, &result, &issue) == SOURCE_PHYSICAL_MISSING);
    assert(!strcmp(result.path, "unchanged") && result.present && issue.path);
    assert(source_physical_resolve(&arena, "generated/tree/new/../regular", SOURCE_PHYSICAL_ENDPOINT, true, &result, &issue) == SOURCE_PHYSICAL_MISSING);
    assert(!strcmp(result.path, "unchanged")); finish();
}
/** Symlinks are refused before '..'; regular files never become intermediate directories. */
static void it_refuses_alias_special_file_and_directory_shape_mismatches(void) {
    prepare(); SourcePhysicalIssue issue;
    const char *paths[] = {"generated/tree/link", "generated/tree/link/../regular", "generated/tree/broken",
        "generated/tree/pipe", "generated/tree/regular/", "generated/tree/regular/../directory"};
    for (size_t index = 0; index < sizeof(paths) / sizeof(*paths); index++) {
        SourcePhysicalPath result = {.path = "unchanged"};
        assert(source_physical_resolve(&arena, paths[index], SOURCE_PHYSICAL_ENDPOINT, true, &result, &issue) == SOURCE_PHYSICAL_UNSUPPORTED);
        assert(!strcmp(result.path, "unchanged") && issue.path);
    }
    SourcePhysicalPath result = {.path = "unchanged"};
    assert(source_physical_resolve(&arena, "generated/tree/regular", SOURCE_PHYSICAL_DIRECTORY, false, &result, &issue) == SOURCE_PHYSICAL_UNSUPPORTED);
    assert(!strcmp(result.path, "unchanged")); finish();
}
/** Each path/cwd/join/parent allocation refusal preserves the caller's prior observation. */
static void it_retires_every_physical_path_allocation_failure_without_partial_output(void) {
    prepare(); SourcePhysicalPath result; SourcePhysicalIssue issue; size_t before = allocation_count;
    assert(source_physical_resolve(&arena, "generated/tree/directory/../regular", SOURCE_PHYSICAL_ENDPOINT, false, &result, &issue) == SOURCE_PHYSICAL_OK);
    size_t count = allocation_count - before; finish(); assert(count);
    for (size_t ordinal = 1; ordinal <= count; ordinal++) {
        prepare(); failure_at = allocation_count + ordinal; result = (SourcePhysicalPath){.path = "unchanged", .present = true};
        assert(source_physical_resolve(&arena, "generated/tree/directory/../regular", SOURCE_PHYSICAL_ENDPOINT, false, &result, &issue) == SOURCE_PHYSICAL_STORAGE);
        assert(!strcmp(result.path, "unchanged") && result.present); failure_at = 0; finish();
    }
    prepare(); result = (SourcePhysicalPath){.path = "unchanged"};
    assert(source_physical_resolve(&arena, "", SOURCE_PHYSICAL_ENDPOINT, false, &result, &issue) == SOURCE_PHYSICAL_INVALID);
    assert(source_physical_resolve(NULL, ".", SOURCE_PHYSICAL_ENDPOINT, false, &result, &issue) == SOURCE_PHYSICAL_INVALID);
    assert(source_physical_resolve(&arena, NULL, SOURCE_PHYSICAL_ENDPOINT, false, &result, &issue) == SOURCE_PHYSICAL_INVALID);
    assert(source_physical_resolve(&arena, ".", (SourcePhysicalKind)99, false, &result, &issue) == SOURCE_PHYSICAL_INVALID);
    assert(source_physical_resolve(&arena, ".", SOURCE_PHYSICAL_ENDPOINT, false, NULL, &issue) == SOURCE_PHYSICAL_INVALID);
    assert(source_physical_resolve(&arena, ".", SOURCE_PHYSICAL_ENDPOINT, false, &result, NULL) == SOURCE_PHYSICAL_INVALID);
    assert(!strcmp(result.path, "unchanged")); finish();
}
/** Require controlled child completion; signal termination is never a passing OS case. */
static void collected(pid_t child) {
    int status; assert(waitpid(child, &status, 0) == child && WIFEXITED(status) && WEXITSTATUS(status) == 0);
}
/** Long actual cwd exercises several observed ERANGE growth steps and every allocation. */
static void it_grows_for_actual_long_cwd_and_unwinds_each_capacity_failure(void) {
    char deep[1024] = "generated/tree";
    for (size_t index = 0; index < 8; index++) {
        size_t length = strlen(deep); const char *part = "/a deliberately long physical directory component";
        assert(length + strlen(part) < sizeof(deep)); strcat(deep, part); directory(deep);
    }
    char *expected = realpath(deep, NULL); assert(expected && strlen(expected) > 256);
    pid_t child = fork(); assert(child >= 0);
    if (!child) {
        assert(chdir(expected) == 0); prepare(); SourcePhysicalPath result; SourcePhysicalIssue issue;
        assert(source_physical_resolve(&arena, ".", SOURCE_PHYSICAL_DIRECTORY, false, &result, &issue) == SOURCE_PHYSICAL_OK);
        assert(!strcmp(result.path, expected)); size_t count = allocation_count; finish(); assert(count >= 4);
        for (size_t ordinal = 1; ordinal <= count; ordinal++) {
            prepare(); failure_at = ordinal; result = (SourcePhysicalPath){.path = "unchanged"};
            assert(source_physical_resolve(&arena, ".", SOURCE_PHYSICAL_DIRECTORY, false, &result, &issue) == SOURCE_PHYSICAL_STORAGE);
            assert(!strcmp(result.path, "unchanged")); failure_at = 0; finish();
        }
        _exit(0);
    }
    collected(child); free(expected);
}
/** Deleted cwd is a real host failure, not a reason to infer PWD or silently choose root. */
static void it_refuses_unavailable_actual_cwd_without_fallback(void) {
    directory("generated/gone"); char *path = realpath("generated/gone", NULL); assert(path);
    pid_t child = fork(); assert(child >= 0);
    if (!child) {
        assert(chdir(path) == 0 && rmdir(path) == 0); prepare();
        SourcePhysicalPath result = {.path = "unchanged"}; SourcePhysicalIssue issue;
        assert(source_physical_resolve(&arena, ".", SOURCE_PHYSICAL_ENDPOINT, false, &result, &issue) == SOURCE_PHYSICAL_IO_ERROR);
        assert(!strcmp(result.path, "unchanged") && !issue.path); finish(); _exit(0);
    }
    collected(child); free(path);
}
#include "destination.c"
/** Execute exact physical traversal and actual host/allocation refusal partitions. */
void run(void) {
    fixtures();
    it_matches_existing_directory_file_and_raw_byte_paths_against_posix();
    it_distinguishes_missing_final_destinations_from_missing_intermediate_traversal();
    it_refuses_alias_special_file_and_directory_shape_mismatches();
    it_retires_every_physical_path_allocation_failure_without_partial_output();
    it_grows_for_actual_long_cwd_and_unwinds_each_capacity_failure();
    it_refuses_unavailable_actual_cwd_without_fallback();
    it_normalizes_missing_destination_tails_without_relaxing_source_traversal();
    it_refuses_unverified_destination_navigation_aliases_and_wrong_kinds();
    it_retires_destination_proposal_failures_and_preserves_invalid_outputs();
}
/** Preserve generated evidence on failed assertions. */
int main(void) { run(); return 0; }
