/**
 * @file
 * @brief Materialize normalized bytes and component provenance in one aligned arena block.
 * [SRC-06](../../../../spec/language/source-and-names.md#src-06) preserves exact
 * root/component bytes; [ARCH-03](../../../../spec/implementation/compiler.md#arch-03)
 * retains scratch ownership on refusal. [ARCH-05](../../../../spec/implementation/compiler.md#arch-05)
 * checks every count/text/terminator sum before publication.
 */
#include <stdint.h>
#include <string.h>
#include "internal.h"

/** Flexible component storage supplies alignment without a guessed header offset.
 * [ARCH-05](../../../../spec/implementation/compiler.md#arch-05) requires the
 * compiler-provided member offset, not an assumption about struct padding.
 */
typedef struct {
    ModulePathValue value;
    ModulePathPart parts[];
} PathStorage;

ModulePathStatus path_normalization_store(SourceArena *arena, ModuleRootKind root,
    const ModulePathPart *parts, size_t count, SourceSpan expression,
    const ModulePathValue **output, SourceIssue *issue) {
    if (root != MODULE_ROOT_PROJECT && root != MODULE_ROOT_STANDARD) {
        return path_normalization_failure(issue, expression, MODULE_PATH_INVALID,
            "Invalid path namespace family", "an ordinary or standard namespace");
    }
    size_t header = offsetof(PathStorage, parts);
    if (count > (SIZE_MAX - header) / sizeof(ModulePathPart)) {
        return path_normalization_failure(issue, expression, MODULE_PATH_LIMIT,
            "Path result storage is not representable", "a representable component count");
    }
    size_t length = root == MODULE_ROOT_STANDARD ? 2 : 1;
    for (size_t index = 0; index < count; index++) {
        if (index != 0) {
            if (length == SIZE_MAX) { goto limit; }
            length++;
        }
        if (parts[index].length > SIZE_MAX - length) { goto limit; }
        length += parts[index].length;
    }
    size_t storage_size = header + count * sizeof(ModulePathPart);
    if (length == SIZE_MAX || storage_size > SIZE_MAX - length - 1) { goto limit; }
    size_t allocation_size = storage_size + length + 1;
    if (allocation_size < sizeof(PathStorage)) { allocation_size = sizeof(PathStorage); }
    PathStorage *storage = source_arena_allocate(arena, allocation_size);
    if (storage == NULL) {
        return path_normalization_failure(issue, expression, MODULE_PATH_OUT_OF_MEMORY,
            "Normalized path allocation failed", "available normalized path storage");
    }
    char *text = (char *)storage + storage_size;
    size_t cursor = root == MODULE_ROOT_STANDARD ? 2 : 1;
    text[0] = '/';
    if (root == MODULE_ROOT_STANDARD) { text[1] = '/'; }
    for (size_t index = 0; index < count; index++) {
        if (index != 0) { text[cursor++] = '/'; }
        memcpy(text + cursor, parts[index].bytes, parts[index].length);
        storage->parts[index] = (ModulePathPart){(const unsigned char *)text + cursor,
            parts[index].length, parts[index].origin};
        cursor += parts[index].length;
    }
    text[cursor] = 0;
    storage->value = (ModulePathValue){root, count == 0 ? NULL : storage->parts,
        count, text, length, expression};
    *output = &storage->value;
    return MODULE_PATH_OK;
limit:
    return path_normalization_failure(issue, expression, MODULE_PATH_LIMIT,
        "Normalized path size is not representable", "a representable path byte length");
}
