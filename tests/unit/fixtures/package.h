/**
 * @file
 * @brief Prepare explicitly named test-vendor packages for real compiler consumers.
 *
 * This fixture factory creates the fixed test vendor and v1.0.0 version used by
 * the owning cases. It never infers identities from source text or directory
 * contents. [PKG-01](../../../spec/project/packages.md#pkg-01) supplies the shape;
 * production selection/scaffolding do not call this test-only constructor.
 */
#ifndef AMA_TEST_FIXTURES_PACKAGE_H
#define AMA_TEST_FIXTURES_PACKAGE_H
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

/** Create an exact fixture directory or accept that existing directory only.
 * Its parent must exist. Symlinks/files and OS refusal are fixture failures,
 * never successful product behavior or an alternate source namespace.
 */
static inline void fixture_package_directory(const char *path) {
    int made = mkdir(path, 0700);
    assert(made == 0 || errno == EEXIST);
    struct stat metadata;
    assert(lstat(path, &metadata) == 0 && S_ISDIR(metadata.st_mode));
}

/** Format the complete known package location into its independently sized buffer.
 * Inputs are borrowed original fixture bytes; failed formatting aborts setup
 * rather than shortening a path or changing a test's intended identity.
 */
static inline void fixture_package_location(char *path, size_t capacity,
    const char *root, const char *package, const char *suffix) {
    int written = snprintf(path, capacity, "%s/packages/test/%s%s", root, package, suffix);
    assert(written > 0 && (size_t)written < capacity);
}

/** Create one local test/package/v1.0.0 tree with empty metadata and source/test areas.
 * The root's parent exists, and root/package are nonempty trusted fixture strings.
 * Package is one component. Existing directories are reused; package.ini is
 * deliberately replaced by an empty file before the compiler borrows it.
 * malloc storage is fixture-owned, outside counted production allocation; it is
 * released before return. Each fixed suffix is visited once. Checked size sums
 * impose no arbitrary root-path cap; filesystem limits remain setup failures.
 */
static inline void fixture_package_prepare(const char *root, const char *package) {
    assert(root != NULL && root[0] != 0 && package != NULL && package[0] != 0);
    assert(strchr(package, '/') == NULL);
    size_t root_length = strlen(root), package_length = strlen(package);
    size_t overhead = sizeof("/packages/test//v1.0.0/package.ini");
    assert(package_length <= SIZE_MAX - overhead);
    assert(root_length <= SIZE_MAX - overhead - package_length);
    size_t capacity = root_length + package_length + overhead;
    char *path = malloc(capacity);
    assert(path != NULL);
    fixture_package_directory(root);
    const char *parents[] = {"/packages", "/packages/test"};
    for (size_t index = 0; index < 2; index++) {
        int written = snprintf(path, capacity, "%s%s", root, parents[index]);
        assert(written > 0 && (size_t)written < capacity);
        fixture_package_directory(path);
    }
    const char *areas[] = {"", "/v1.0.0", "/v1.0.0/src", "/v1.0.0/tests"};
    for (size_t index = 0; index < 4; index++) {
        fixture_package_location(path, capacity, root, package, areas[index]);
        fixture_package_directory(path);
    }
    fixture_package_location(path, capacity, root, package, "/v1.0.0/package.ini");
    FILE *configuration = fopen(path, "wb");
    assert(configuration != NULL);
    assert(fclose(configuration) == 0);
    free(path);
}
#endif
