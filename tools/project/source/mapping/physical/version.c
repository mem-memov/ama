/** @file
 * @brief Reject ambiguous destination-version tables before using any entry.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * forbids discovery-order ownership; duplicate version descriptors therefore
 * refuse even when one requested entry could be found earlier in the table.
 */
#include "version.h"
#include "source/path/logical.h"
#include <string.h>
bool source_version_placements_valid(const SourceVersionPlacement *versions, size_t count) {
    if (count && !versions) { return false; }
    for (size_t i = 0; i < count; i++) {
        ModulePathShape shape;
        if (!versions[i].identity || !versions[i].physical || versions[i].physical[0] != '/'
            || !module_path_describe((const unsigned char *)versions[i].identity,
                strlen(versions[i].identity), &shape)
            || shape.root != MODULE_ROOT_PROJECT || shape.extent != MODULE_PATH_VERSION) { return false; }
        for (size_t j = 0; j < i; j++) {
            if (!strcmp(versions[i].identity, versions[j].identity)) { return false; }
        }
    }
    return true;
}
