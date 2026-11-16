/**
 * @file
 * @brief Prepare explicit absolute output/SDK paths and verify required SDK headers.
 *
 * [CABI-06](../../../../spec/implementation/native-interface.md#cabi-06)
 * Reject paths not representable in a C header name; no guessed output directory or hidden include is supplied.
 */
#define _XOPEN_SOURCE 700
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "native/scaffold/internal.h"

/** Own a getcwd buffer in the arena, growing only after ERANGE with checked doubling; preserve the first IO/allocation error.
 * Component contract: [CABI-06](../../../../spec/implementation/native-interface.md#cabi-06). */
static const char *working_directory(NativeScaffold *scaffold) {
    size_t capacity = 128;
    for (;;) {
        char *buffer = scaffold_allocate(scaffold, capacity);
        if (buffer == NULL) {
            return NULL;
        }
        if (getcwd(buffer, capacity) != NULL) {
            return buffer;
        }
        if (errno != ERANGE || capacity > SIZE_MAX / 2) {
            scaffold_error(scaffold, DIAGNOSTIC_IO_FAILURE, "Cannot determine the working directory", NULL);
            return NULL;
        }
        capacity *= 2;
    }
}

const char *scaffold_absolute(NativeScaffold *scaffold, const char *path) {
    if (path == NULL || *path == 0) {
        scaffold_error(scaffold, DIAGNOSTIC_INVALID_PATH, "An explicit output/SDK path is required", NULL);
        return NULL;
    }
    const char *absolute = scaffold_join(scaffold, *path == '/' ? "" : scaffold->working_directory,
        *path == '/' ? "" : "/", path);
    if (absolute == NULL) {
        return NULL;
    }
    for (const unsigned char *cursor = (const unsigned char *)absolute; *cursor != 0; cursor++) {
        if (*cursor == '"' || *cursor == '\\' || *cursor < 32 || *cursor == 127) {
            scaffold_error(scaffold, DIAGNOSTIC_UNSUPPORTED_PATH,
                "This physical path cannot be represented as a C header name", absolute);
            return NULL;
        }
    }
    return absolute;
}

bool scaffold_paths(NativeScaffold *scaffold) {
    scaffold->working_directory = working_directory(scaffold);
    if (scaffold->working_directory == NULL) {
        return false;
    }
    scaffold->directory = scaffold_absolute(scaffold, scaffold->options.directory);
    scaffold->sdk = scaffold_absolute(scaffold, scaffold->options.sdk);
    if (scaffold->directory == NULL || scaffold->sdk == NULL) {
        return false;
    }
    const char *required[] = {"native/status/status.h", "native/status/library.h", "native/abi/bytes.h",
        "native/abi/services.h", "native/toolchain/gcc.h"};
    for (size_t index = 0; index < sizeof(required) / sizeof(required[0]); index++) {
        const char *path = scaffold_join(scaffold, scaffold->sdk, "/", required[index]);
        struct stat metadata;
        if (path == NULL || stat(path, &metadata) != 0 || !S_ISREG(metadata.st_mode)) {
            return scaffold_error(scaffold, DIAGNOSTIC_INVALID_PATH, "The selected SDK header is unavailable", path);
        }
    }
    return true;
}
