/** @file
 * @brief Own original fixture captures before exercising independent observation/image preparation.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
#ifndef AMA_SOURCE_OBSERVATION_TEST_COMMON
#define AMA_SOURCE_OBSERVATION_TEST_COMMON
#include "../selection/common.c"
#include "../../../../source/module/packages/support/io.h"
#include "../../../../../../tools/project/source/observation/observation.h"
/** Compare exact original and replayed common source sets without relying on order. */
static void compare_catalog(const ModuleCatalog *first, const ModuleCatalog *second) {
    size_t left = 0, right = 0;
    for (const ModuleSource *source = first->first; source; source = source->next) {
        const ModuleSource *other = NULL;
        assert(module_catalog_find(second, source->logical_path, &other) == MODULE_OK);
        assert(!strcmp(source_path(source->text), source_path(other->text)));
        assert(source_length(source->text) == source_length(other->text));
        assert(!memcmp(source_bytes(source->text), source_bytes(other->text), source_length(source->text)));
        assert(source->device == other->device && source->inode == other->inode); left++;
    }
    for (const ModuleSource *source = second->first; source; source = source->next) { right++; }
    assert(left == right);
}
/** Inspect an exact immutable image entry, including unavailable listing members. */
static const SourceCatalogImageEntry *observed_entry(const SourceObservedImage *observed, const char *suffix) {
    size_t length = strlen(suffix);
    for (size_t i = 0; i < observed->image.count; i++) {
        size_t path = strlen(observed->image.entries[i].path);
        if (path >= length && !strcmp(observed->image.entries[i].path + path - length, suffix)) {
            return &observed->image.entries[i];
        }
    }
    return NULL;
}
#endif
