/** @file
 * @brief Rebuild an owned module registry from authoritative captured source bytes without filesystem rewriting.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * resolves original identities before edits;
 * [SRC-02](../../../../spec/language/source-and-names.md#src-02)
 * forbids duplicate logical names or physical aliases in the analysis registry.
 * [PKG-03](../../../../spec/project/packages.md#pkg-03) captures optional module
 * configuration, including explicit absence, without physical rereads.
 */
#include <stdint.h>
#include <string.h>
#include "../internal.h"
#include "source/unicode/unicode.h"

/** Both complete logical identity and captured physical identity must remain unique. Component contract: [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02), [SRC-02](../../../../spec/language/source-and-names.md#src-02). */
static ModuleStatus unique(ModuleCatalog *catalog, const ModuleSnapshotEntry *entry, const char *physical) {
    for (const ModuleSource *other = catalog->first; other != NULL; other = other->next) {
        bool same_name = strcmp(entry->logical_path,other->logical_path) == 0;
        if (same_name || (entry->device == other->device && entry->inode == other->inode)) {
            catalog->issue.related_path = source_path(other->text);
            return module_fail(catalog,same_name ? MODULE_AMBIGUOUS : MODULE_FILE_ALIAS,physical);
        }
    }
    return MODULE_OK;
}

/** Copy one complete entry before linking it; failed bytes and diagnostics stay catalog-owned. Component contract: [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02), [SRC-02](../../../../spec/language/source-and-names.md#src-02). */
static ModuleStatus insert(ModuleCatalog *catalog, const ModuleSnapshotEntry *entry) {
    if (entry->text == NULL || !module_path_valid(entry->logical_path)) {
        return module_fail(catalog,MODULE_INVALID_PATH,NULL);
    }
    const char *physical = source_arena_copy(&catalog->arena,source_path(entry->text));
    if (physical == NULL) { return module_fail(catalog,MODULE_OUT_OF_MEMORY,NULL); }
    ModuleStatus status = unique(catalog,entry,physical);
    if (status != MODULE_OK) { return status; }
    ModuleSource *source = source_arena_allocate(&catalog->arena,sizeof(*source));
    const char *logical = source_arena_copy(&catalog->arena,entry->logical_path);
    if (source == NULL || logical == NULL) { return module_fail(catalog,MODULE_OUT_OF_MEMORY,physical); }
    *source = (ModuleSource){.logical_path=logical,.device=entry->device,.inode=entry->inode};
    SourceStatus copied = source_manager_add(&catalog->sources,physical,
        source_bytes(entry->text),source_length(entry->text),&source->text);
    if (copied != SOURCE_OK) { return module_fail(catalog,MODULE_OUT_OF_MEMORY,physical); }
    size_t offset;
    if (!utf8_validate(source_bytes(source->text),source_length(source->text),&offset)) {
        catalog->issue.source = source->text;
        catalog->issue.offset = offset;
        return module_fail(catalog,MODULE_INVALID_UTF8,source_path(source->text));
    }
    ModuleStatus configuration = module_snapshot_aliases(catalog, source, entry->configuration);
    if (configuration != MODULE_OK) { return configuration; }
    source->next = catalog->first;
    catalog->first = source;
    return MODULE_OK;
}

/** Snapshot rows borrow only the build call; no partial registry is exposed as built after failure. Component contract: [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02), [SRC-02](../../../../spec/language/source-and-names.md#src-02). */
ModuleStatus module_catalog_snapshot(ModuleCatalog *catalog, const ModuleSnapshotEntry *entries, size_t count) {
    if (catalog->attempted || (count != 0 && entries == NULL)) { return MODULE_INVALID_PATH; }
    catalog->attempted = true;
    if (count > SIZE_MAX/sizeof(ModuleSource)) { return module_fail(catalog,MODULE_OUT_OF_MEMORY,NULL); }
    for (size_t index = 0; index < count; index++) {
        ModuleStatus status = insert(catalog,&entries[index]);
        if (status != MODULE_OK) { return status; }
    }
    catalog->built = true;
    return MODULE_OK;
}
