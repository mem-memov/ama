/**
 * @file
 * @brief Create, copy and remove only test-owned filesystem fixtures with asserted IO completion.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../docs/implementation/library-tests-and-build.md)
 */
#ifndef AMA_TEST_INTEGRATION_FILES_C
#define AMA_TEST_INTEGRATION_FILES_C

#include <dirent.h>
#include <sys/stat.h>

/**
 * Create one expected test-owned directory and require success rather than silently ignoring
 * unexpected filesystem state.
 */
static void make_directory(const char *path) {
    int result = mkdir(path, 0700);
    assert(result == 0);
}

/**
 * Recursively remove only the finite test-owned fixture tree, use lstat rather than following
 * links and assert every path/IO boundary. This cleanup is not a production module traversal
 * algorithm.
 */
static void remove_tree(const char *path) {
    DIR *directory = opendir(path);
    assert(directory != NULL);
    struct dirent *entry;
    while ((entry = readdir(directory)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        char child[4096];
        int length = snprintf(child, sizeof(child), "%s/%s", path, entry->d_name);
        assert(length > 0 && (size_t) length < sizeof(child));
        struct stat information;
        int result = lstat(child, &information);
        assert(result == 0);
        if (S_ISDIR(information.st_mode)) {
            remove_tree(child);
        } else {
            result = unlink(child);
            assert(result == 0);
        }
    }
    int result = closedir(directory);
    assert(result == 0);
    result = rmdir(path);
    assert(result == 0);
}

/**
 * Write a complete explicit fixture file and check final close; callers provide a test-owned
 * path and valid content.
 */
static void write_file(const char *path, const char *contents) {
    FILE *file = fopen(path, "w");
    assert(file != NULL);
    int result = fputs(contents, file);
    assert(result != EOF);
    result = fclose(file);
    assert(result == 0);
}

/**
 * Copy complete bytes between fixture paths, checking reads, writes and closes; neither input
 * nor destination is retained afterward.
 */
static void copy_file(const char *source, const char *destination) {
    FILE *input = fopen(source, "rb");
    FILE *output = fopen(destination, "wb");
    assert(input != NULL && output != NULL);
    char buffer[4096];
    size_t length;
    while ((length = fread(buffer, 1, sizeof(buffer), input)) != 0) {
        size_t written = fwrite(buffer, 1, length, output);
        assert(written == length);
    }
    assert(!ferror(input));
    int result = fclose(input);
    assert(result == 0);
    result = fclose(output);
    assert(result == 0);
}

#endif
