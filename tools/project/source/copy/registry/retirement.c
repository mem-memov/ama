/** @file
 * @brief Compare actual selected main provenance without inventing physical deletions.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * preserves original and copied binding laws even for same-identity promotion;
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * separates that registry transition from the original physical removal union.
 */
#include <stdint.h>
#include <string.h>
#include "retirement.h"
/** Exact checked catalogs have one unit for each complete nominal identity. */
static const SourceCapturedUnit *find(const SourceCapturedUnit *units, const char *name) {
    while (units && strcmp(units->source->logical_path, name)) { units = units->next; }
    return units;
}
/** Selection retirement includes reviewed physical removals, never conversely
 * grants them; compare original/final selected paths for the remaining names. */
static bool retired(const SourceCapturedUnit *old, const SourceImageCapture *final,
    const char *const *removed, size_t count) {
    const char *name = old->source->logical_path;
    for (size_t i = 0; i < count; i++) { if (!strcmp(name, removed[i])) { return true; } }
    const SourceCapturedUnit *now = find(final->units, name);
    return !now || strcmp(source_path(old->source->text), source_path(now->source->text));
}
SourceRegistryStatus source_copy_registry_retirement(SourceArena *arena, const SourceRootCapture *original,
    const SourceImageCapture *final, const char *const *removed, size_t count, SourceRegistryRetirement *output) {
    if (!arena || !original || !original->valid || !final || !final->valid || !output || (count && !removed)) { return SOURCE_REGISTRY_INVALID; }
    for (size_t i = 0; i < count; i++) {
        if (!removed[i] || !find(original->units, removed[i])) { return SOURCE_REGISTRY_INVALID; }
    }
    size_t size = 0;
    for (const SourceCapturedUnit *old = original->units; old; old = old->next) {
        if (retired(old, final, removed, count)) { if (size == SIZE_MAX) { return SOURCE_REGISTRY_MEMORY; } size++; }
    }
    if (size > SIZE_MAX / sizeof(char *)) { return SOURCE_REGISTRY_MEMORY; }
    const char **names = size ? source_arena_allocate(arena, size * sizeof(*names)) : NULL;
    if (size && !names) { return SOURCE_REGISTRY_MEMORY; }
    size_t i = 0;
    for (const SourceCapturedUnit *old = original->units; old; old = old->next) {
        if (retired(old, final, removed, count)) { names[i++] = old->source->logical_path; }
    }
    *output = (SourceRegistryRetirement){names, size}; return SOURCE_REGISTRY_OK;
}
