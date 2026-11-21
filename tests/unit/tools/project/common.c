/** @file
 * @brief Own test directories and counted allocation across independent tool harnesses.
 * @see [T233 contract](../../../../docs/implementation/project-tool.md)
 */
#ifndef AMA_PROJECT_TOOL_TEST_COMMON
#define AMA_PROJECT_TOOL_TEST_COMMON
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include "../../../../tools/project/init/init.h"
#include "../../../../tools/project/init/content.h"
#include "../../../../tools/project/command/run.h"
static char root[128], destination[160];
static size_t allocations, fail_allocation, live;
/** Fail exactly one selected allocation and count every live owned block. */
static void *allocate(void *context, size_t size) {
    (void)context;
    if (++allocations == fail_allocation) { return NULL; }
    void *value = malloc(size);
    if (value) { live++; }
    return value;
}
/** Require balanced ownership and release a block or the permitted null. */
static void release(void *context, void *value) {
    (void)context;
    if (value) { assert(live); live--; free(value); }
}
/** Supply fixture-local allocation authority without global production state. */
static inline SourceAllocator allocator(void) { return (SourceAllocator){NULL, allocate, release}; }
/** Own a fresh generated tree and reset the failure-injection domain. */
static void setup(void) {
    strcpy(root, "generated/init-XXXXXX");
    char *created = mkdtemp(root); assert(created);
    (void)snprintf(destination, sizeof(destination), "%s/project", root);
    allocations = fail_allocation = live = 0;
}
/** Remove a test-owned finite tree without following links. */
static void erase(const char *path) {
    struct stat metadata;
    int read = lstat(path, &metadata);
    if (read) { assert(errno == ENOENT); return; }
    if (S_ISDIR(metadata.st_mode)) {
        DIR *directory = opendir(path); assert(directory);
        struct dirent *entry;
        while ((entry = readdir(directory))) {
            if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) { continue; }
            char child[512];
            int length = snprintf(child, sizeof(child), "%s/%s", path, entry->d_name);
            assert(length > 0 && (size_t)length < sizeof(child));
            erase(child);
        }
        int closed = closedir(directory); assert(!closed);
        int removed = rmdir(path); assert(!removed);
    } else { int removed = unlink(path); assert(!removed); }
}
/** Require complete memory release before removing the test-owned tree. */
static void destroy(void) { assert(live == 0); erase(root); }
/** Count only actual children, independently checking exact scaffold membership. */
static size_t members(const char *path) {
    DIR *directory = opendir(path); assert(directory);
    size_t count = 0; struct dirent *entry;
    while ((entry = readdir(directory))) {
        if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) { count++; }
    }
    int closed = closedir(directory); assert(!closed); return count;
}
/** Create controlled fixture bytes and assert complete write/close. */
static inline void write_file(const char *path, const char *text) {
    FILE *file = fopen(path, "wb"); assert(file);
    size_t written = fwrite(text, 1, strlen(text), file); assert(written == strlen(text));
    int closed = fclose(file); assert(!closed);
}
/** Require byte equality and EOF, rejecting unnoticed trailing diagnostics. */
static inline void assert_file(const char *path, const char *text) {
    FILE *file = fopen(path, "rb"); assert(file);
    for (size_t index = 0; text[index]; index++) { int byte = fgetc(file); assert(byte == (unsigned char)text[index]); }
    int end = fgetc(file); assert(end == EOF);
    int closed = fclose(file); assert(!closed);
}
#endif
