/** @file
 * @brief Hold canonical parent identities before relocation can replace or retire their children.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * applies external importer/configuration/output changes under the same exclusive operation.
 */
#include <string.h>
#include "../internal.h"
/** Canonical absolute input paths have a verified existing parent; root is its own parent. */
bool relocation_transaction_lease_parent(RelocationTransaction *transaction, const char *path) {
    char *parent = source_arena_copy(&transaction->arena,path);
    if (parent == NULL) { return relocation_transaction_fail(transaction,RELOCATION_OUT_OF_MEMORY,"Relocation parent lease allocation failed",path); }
    char *slash = strrchr(parent,'/');
    if (slash == NULL) { return relocation_transaction_fail(transaction,RELOCATION_INVALID_PATH,"A relocation lease path is not absolute",parent); }
    if (slash == parent) { slash[1] = 0; } else { *slash = 0; }
    return relocation_transaction_lease_directory(transaction,parent);
}
