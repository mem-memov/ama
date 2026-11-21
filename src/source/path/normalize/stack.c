/**
 * @file
 * @brief Size scratch storage from validated whole path components without a depth cap.
 * [SRC-06](../../../../spec/language/source-and-names.md#src-06) gives nonempty
 * component progress; [ARCH-05](../../../../spec/implementation/compiler.md#arch-05)
 * requires checked combined count and array-size arithmetic before allocation.
 */
#include <stdint.h>
#include "internal.h"

size_t path_fragment_count(ModulePathFragment fragment) {
    if (fragment.length == 0) { return 0; }
    size_t count = 1;
    for (size_t index = 0; index < fragment.length; index++) {
        if (fragment.bytes[index] == '/') { count++; }
    }
    return count;
}

ModulePathStatus path_normalization_stack(SourceArena *arena, size_t prefix_count,
    ModulePathFragment fragment, ModulePathPart **output, SourceIssue *issue) {
    size_t suffix_count = path_fragment_count(fragment);
    if (suffix_count > SIZE_MAX - prefix_count
        || prefix_count + suffix_count > SIZE_MAX / sizeof(ModulePathPart)) {
        return path_normalization_failure(issue, fragment.origin, MODULE_PATH_LIMIT,
            "Path component storage is not representable", "a representable component count");
    }
    size_t count = prefix_count + suffix_count;
    ModulePathPart *parts = count == 0 ? NULL : source_arena_allocate(arena, count * sizeof(*parts));
    if (count != 0 && parts == NULL) {
        return path_normalization_failure(issue, fragment.origin, MODULE_PATH_OUT_OF_MEMORY,
            "Path stack allocation failed", "available component storage");
    }
    *output = parts;
    return MODULE_PATH_OK;
}
