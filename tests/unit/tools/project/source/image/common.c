/** @file
 * @brief Compare the prepared-image adapter with an independent finite reader model.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
#ifndef AMA_SOURCE_CATALOG_IMAGE_TEST_COMMON
#define AMA_SOURCE_CATALOG_IMAGE_TEST_COMMON
#include "../../../../source/module/view/common.c"
#include "../../../../../../tools/project/source/image/image.h"
/** Snapshot fixture metadata/content independently of production image callbacks.
 * The ignored dependency version has known listing membership but unread metadata;
 * opaque native C/h bytes are also deliberately unavailable to the text reader.
 */
static void image_rows(const ViewModel *model, SourceCatalogImageEntry rows[VIEW_NODE_COUNT + 1]) {
    for (size_t i = 0; i < VIEW_NODE_COUNT; i++) {
        rows[i] = (SourceCatalogImageEntry){.path = nodes[i].path,
            .metadata = nodes[i].directory ? model->directory : model->regular,
            .content = nodes[i].directory ? NULL : model->texts[i],
            .metadata_captured = true, .children_complete = nodes[i].directory};
        rows[i].metadata.st_dev = 1; rows[i].metadata.st_ino = (ino_t)(i + 1);
    }
    rows[12].content = rows[13].content = NULL;
    rows[VIEW_NODE_COUNT] = (SourceCatalogImageEntry){.path = "/virtual/dependencies/vendor/pkg/v1.0.0"};
}
static const ModuleRoot image_roots[] = {{"/virtual", NULL, 0, MODULE_ROOT_PROJECT}, {"/standard", NULL, 0, MODULE_ROOT_STANDARD}};
/** Compare complete selected main sets and exact bytes, independently of linked
 * insertion order. Common catalog owners retain their own new physical-key text. */
static void same_catalog(const ModuleCatalog *first, const ModuleCatalog *second) {
    size_t left = 0, right = 0;
    for (const ModuleSource *source = first->first; source; source = source->next) {
        const ModuleSource *other = NULL;
        assert(module_catalog_find(second, source->logical_path, &other) == MODULE_OK);
        assert(source_length(source->text) == source_length(other->text));
        assert(!memcmp(source_bytes(source->text), source_bytes(other->text), source_length(source->text)));
        assert(!strcmp(source_path(source->text), source_path(other->text)));
        assert(source->device == other->device && source->inode == other->inode); left++;
    }
    for (const ModuleSource *source = second->first; source; source = source->next) { right++; }
    assert(left == right && left == 4);
}
#endif
