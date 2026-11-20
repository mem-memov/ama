/**
 * @file
 * @brief Inject one exact directory-I/O failure while checking real stream ownership.
 *
 * Wrappers delegate to the real filesystem except for one selected operation;
 * close injection reports failure after real release so cleanup is independently
 * observable. This fixture is single-threaded and never alters production code.
 * @see [T258 evidence](../../../../../../docs/implementation/versioned-source.md)
 */
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include "io.h"

/** Measured scope and operation ordinal belong only to this test executable. */
static bool enabled;
/** Zero disables failure, while a positive ordinal fails exactly once. */
static size_t failure_ordinal;
/** Count all wrapped calls during the currently enabled scope. */
static size_t operation_count;
/** Count successfully acquired directory streams not yet actually closed. */
static size_t live_streams;

/** Linker-provided actual directory opener; ownership transfers on nonnull result. */
DIR *__real_opendir(const char *path);
/** Linker-provided actual directory iterator; result borrows the stream lifetime. */
struct dirent *__real_readdir(DIR *stream);
/** Linker-provided actual directory closer; zero confirms stream release. */
int __real_closedir(DIR *stream);
/** Linker-provided actual nonsymlink-following metadata probe. */
int __real_lstat(const char *path, struct stat *metadata);

/** Count one operation and select only the requested ordinal without timing assumptions. */
static bool fails(void) {
    if (!enabled) {
        return false;
    }
    assert(operation_count != SIZE_MAX);
    operation_count++;
    return failure_ordinal != 0 && operation_count == failure_ordinal;
}

/** Refuse one acquisition before ownership exists; otherwise count its real stream. */
DIR *__wrap_opendir(const char *path) {
    if (fails()) {
        errno = EIO;
        return NULL;
    }
    DIR *stream = __real_opendir(path);
    if (enabled && stream != NULL) {
        live_streams++;
    }
    return stream;
}

/** Fail iteration with a real I/O error rather than silently reporting EOF. */
struct dirent *__wrap_readdir(DIR *stream) {
    if (fails()) {
        errno = EIO;
        return NULL;
    }
    return __real_readdir(stream);
}

/** Always release the real stream, then optionally report the selected close failure. */
int __wrap_closedir(DIR *stream) {
    bool fail = fails();
    int result = __real_closedir(stream);
    if (enabled && result == 0) {
        assert(live_streams != 0);
        live_streams--;
    }
    if (fail) {
        errno = EIO;
        return -1;
    }
    return result;
}

/** Refuse one metadata request without publishing a fabricated stat result. */
int __wrap_lstat(const char *path, struct stat *metadata) {
    if (fails()) {
        errno = EIO;
        return -1;
    }
    return __real_lstat(path, metadata);
}

/** Reset one explicit scope; fixture setup and teardown remain outside injection. */
void fixture_io_begin(size_t ordinal) {
    assert(!enabled && live_streams == 0);
    operation_count = 0;
    failure_ordinal = ordinal;
    enabled = true;
}

/** Finish only after every acquired directory stream has been released. */
size_t fixture_io_end(void) {
    assert(enabled && live_streams == 0);
    enabled = false;
    return operation_count;
}
