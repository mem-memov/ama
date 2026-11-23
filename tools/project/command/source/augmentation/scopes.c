/** @file
 * @brief Represent the union of equal-identity root scopes without indexing the same physical root twice.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * requires new coverage while preserving existing root-based identities.
 */
#include "scopes.h"
#include <string.h>
/** Relative canonical component keys denote subtrees; byte prefixes alone do not. */
static bool below(const char *path, const char *root) {
    size_t length = strlen(root);
    return !strncmp(path, root, length) && (!path[length] || path[length] == '/');
}
ProjectSourceAugmentationStatus project_source_augmentation_scope_union(SourceArena *arena, ModuleRoot *target, const ModuleRoot *added) {
    if (!arena || !target || !added || !target->path || !added->path
        || (target->excluded_count && !target->excluded) || (added->excluded_count && !added->excluded)
        || strcmp(target->path, added->path) || target->kind != added->kind) { return PROJECT_SOURCE_AUGMENTATION_INVALID; }
    size_t a = target->excluded_count, b = added->excluded_count;
    if (a && b > SIZE_MAX / a) { return PROJECT_SOURCE_AUGMENTATION_LIMIT; }
    size_t capacity = a * b;
    if (capacity > SIZE_MAX / sizeof(const char *)) { return PROJECT_SOURCE_AUGMENTATION_LIMIT; }
    const char **keys = capacity ? source_arena_allocate(arena, capacity * sizeof(*keys)) : NULL;
    if (capacity && !keys) { return PROJECT_SOURCE_AUGMENTATION_MEMORY; }
    size_t count = 0;
    for (size_t i = 0; i < a; i++) {
        for (size_t j = 0; j < b; j++) {
            const char *key = below(target->excluded[i], added->excluded[j]) ? target->excluded[i]
                : below(added->excluded[j], target->excluded[i]) ? added->excluded[j] : NULL;
            if (!key) { continue; }
            bool repeated = false;
            for (size_t k = 0; k < count; k++) { repeated |= !strcmp(keys[k], key); }
            if (!repeated) { keys[count++] = key; }
        }
    }
    target->excluded = keys; target->excluded_count = count; return PROJECT_SOURCE_AUGMENTATION_OK;
}
