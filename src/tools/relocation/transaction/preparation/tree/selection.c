/** @file
 * @brief Omit only explicitly declared generated output subtrees from the prepared destination tree.
 * [TOOL-03](../../../../../../spec/implementation/tools-and-conformance.md#tool-03)
 * invalidates generated products without guessing artifact filenames or modifying native/assets bytes.
 */
#include "../../internal.h"
#include "../../../internal.h"
/** T112 already rejected protected input overlap; every remaining contained physical entry is explicitly disposable output. */
bool relocation_transaction_invalidated(const RelocationStaging *staging, const char *path) {
    for (const RelocationInvalidation *output = staging->invalidations; output != NULL; output = output->next) {
        if (relocation_contains(output->after,path)) { return true; }
    }
    return false;
}
