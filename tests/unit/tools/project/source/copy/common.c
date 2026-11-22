/** @file
 * @brief Share real admitted original inputs with one owned raw selection and copy-closure preparation.
 * @see [T224 contract](../../../../../../docs/implementation/source-operations.md)
 */
#ifndef AMA_SOURCE_COPY_TEST_COMMON
#define AMA_SOURCE_COPY_TEST_COMMON
#include "../expansion/common.c"
#include "../../../../../../tools/project/source/copy/closure.h"
static SourceTreeSelection tree;
static SourceCopyClosure copy;
/** Read one actually indexed main through exact raw structural capture. */
static void select_main(void) {
    const ModuleSource *main = NULL;
    assert(module_catalog_find(&capture.catalog, main_identity, &main) == MODULE_OK);
    source_tree_selection_init(&tree, sources.allocator);
    assert(source_tree_selection_read(&tree, &capture, source_path(main->text)) == SOURCE_TREE_OK);
}
/** Retire copy borrowers before raw captures and the original compiler domain. */
static void finish_copy(void) {
    source_copy_closure_destroy(&copy); source_tree_selection_destroy(&tree); finish();
}
/** Inspect exact role membership, not an inferred filename prefix. */
static const AcquisitionRoleRecord *copy_role(const char *identity) {
    for (const AcquisitionRoleRecord *role = copy.closure.roles.records; role; role = role->next) {
        if (!strcmp(role->identity, identity)) { return role; }
    }
    return NULL;
}
#endif
