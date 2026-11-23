/** @file
 * @brief Own genuine input captures and disjoint scratch fixtures for workspace composition tests.
 * @see [T287 entry review](../../../../../../docs/implementation/source-operations.md#t287-entry-review-after-accepted-t286)
 */
#ifndef AMA_SOURCE_WORKSPACE_TEST_COMMON
#define AMA_SOURCE_WORKSPACE_TEST_COMMON
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include "../../../../source/common.c"
#include "../../../../../../tools/project/source/workspace/workspace.h"
#include "../../../../../../tools/project/source/capture/capture.h"
static SourceRootCapture captured;
static SourceArena fixture_arena;
static const char *temporary_parent;
/** Exact fixture paths are physical byte strings, never inferred module roots. */
static void directory(const char *path) { assert(mkdir(path, 0700) == 0 || errno == EEXIST); }
/** Use a real ordinary source observation; workspace serialization alone grants
 * no language/compiler authority, and tests never forge owner validity flags. */
static void begin(void) {
    setup_test(); source_arena_init(&fixture_arena, sources.allocator);
    directory("generated/source"); directory("generated/temporary");
    FILE *source = fopen("generated/source/main", "wb"); assert(source);
    assert(fwrite("@!\n", 1, 3, source) == 3 && fclose(source) == 0);
    source_root_capture_init(&captured, sources.allocator);
    ModuleRoot root = {.path = "generated/source", .kind = MODULE_ROOT_STANDARD};
    assert(source_root_capture_build(&captured, &root, 1) == SOURCE_CAPTURE_OK);
    SourcePhysicalPath parent; SourcePhysicalIssue issue;
    assert(source_physical_resolve(&fixture_arena, "generated/temporary", SOURCE_PHYSICAL_DIRECTORY, false, &parent, &issue) == SOURCE_PHYSICAL_OK);
    temporary_parent = parent.path;
}
/** Require exact source bytes and no private child leaks after every case. */
static size_t members(const char *path) {
    DIR *stream = opendir(path); assert(stream); size_t count = 0;
    for (;;) {
        errno = 0; struct dirent *entry = readdir(stream);
        if (!entry) { assert(!errno); break; }
        if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) { count++; }
    }
    assert(closedir(stream) == 0); return count;
}
/** Count actual retained descriptors without including the scan owner itself. */
static size_t descriptors(void) {
    DIR *stream = opendir("/proc/self/fd"); assert(stream); size_t count = 0;
    while (readdir(stream)) { count++; }
    assert(closedir(stream) == 0); return count;
}
/** Filesystem fixture ownership is separate from compiler request ownership. */
static void end(void) {
    assert(members(temporary_parent) == 0 && members(captured.roots[0].path) == 1);
    FILE *source = fopen("generated/source/main", "rb"); assert(source); char bytes[4] = {0};
    assert(fread(bytes, 1, 3, source) == 3 && !memcmp(bytes, "@!\n", 3) && fgetc(source) == EOF && fclose(source) == 0);
    source_root_capture_destroy(&captured); source_arena_destroy(&fixture_arena); destroy_test();
}
/** Compare independently opened physical bytes with the exact immutable record
 * supplied to the shared tree writer, including EOF and current inode ownership. */
static void request_bytes(const SourceWorkspace *workspace, SourceWorkspaceSlot slot) {
    const SourceWorkspaceFile *file = &workspace->files[slot];
    FILE *stream = fopen(file->entry.path, "rb"); assert(stream);
    for (size_t i = 0; i < source_length(file->entry.content); i++) { assert(fgetc(stream) == source_bytes(file->entry.content)[i]); }
    assert(fgetc(stream) == EOF && fclose(stream) == 0);
    struct stat actual;
    assert(fstatat(workspace->directory, file->name, &actual, AT_SYMLINK_NOFOLLOW) == 0
        && source_material_tree_owns(&file->tree, file->entry.path, &actual));
}
#endif
