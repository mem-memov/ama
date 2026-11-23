/** @file
 * @brief Share narrow creation, readback and owned-cleanup state between siblings.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * forbids deleting unrelated material after a failed preparation.
 */
#ifndef AMA_SOURCE_TRANSACTION_TREE_INTERNAL_H
#define AMA_SOURCE_TRANSACTION_TREE_INTERNAL_H
#include "tree.h"
/** A successful exclusive creation owns its descriptor; identity/mode and exact
 * completed byte prefix describe the only material cleanup may remove. */
typedef struct SourceMaterialRow {
    const SourceCatalogImageEntry *expected;
    const char *name;
    size_t parent, written;
    int descriptor;
    struct stat identity;
    bool created, identified, removed;
} SourceMaterialRow;
/** Retain precise first failed operation rather than mapping all errors to absence. */
bool source_material_tree_fail(SourceMaterialTree *, SourceMaterialTreeStatus, const char *, int);
/** Check deterministic preparation cancellation before a host operation. */
bool source_material_tree_permit(SourceMaterialTree *, SourceTreeOperation, const char *);
/** Allocate and validate a complete descriptor-relative material topology. */
bool source_material_tree_shape(SourceMaterialTree *, const SourceCatalogImageEntry *const *, size_t);
/** Create and retain one exact exclusive entry and write regular-file bytes. */
bool source_material_tree_create(SourceMaterialTree *, size_t);
/** Finalize complete file/directory modes and durable contents in reverse order. */
bool source_material_tree_finish(SourceMaterialTree *);
/** Resolve one row's existing retained parent without traversing path strings. */
int source_material_tree_parent(const SourceMaterialTree *, size_t);
#endif
