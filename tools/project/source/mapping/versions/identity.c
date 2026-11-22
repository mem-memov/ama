/** @file
 * @brief Preserve the full additional source/test suffix under a designated version.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * places extra dependencies by complete src-relative suffix, and test helpers
 * outside associations by complete tests-relative suffix, never next to a renamed
 * requesting main. Physical SRC-05 topology is a later independent obligation.
 */
#include "plan.h"
#include "source/path/logical.h"
#include <string.h>
SourceStatus source_additional_module_identity(SourceArena *arena,
    const SourceVersionAssignment *assignment, const char **output) {
    ModulePathShape original, destination;
    if (!arena || !assignment || !assignment->original || !assignment->original->logical_path ||
        !assignment->version || !output ||
        !module_path_describe((const unsigned char *)assignment->original->logical_path,
            strlen(assignment->original->logical_path), &original) ||
        original.root != MODULE_ROOT_PROJECT || original.extent != MODULE_PATH_MODULE ||
        !module_path_describe((const unsigned char *)assignment->version,
            strlen(assignment->version), &destination) ||
        destination.root != MODULE_ROOT_PROJECT || destination.extent != MODULE_PATH_VERSION) {
        return SOURCE_INVALID_ARGUMENT;
    }
    const char *identity = source_arena_join(arena, assignment->version, "",
        assignment->original->logical_path + original.version_length);
    if (!identity) { return SOURCE_OUT_OF_MEMORY; }
    *output = identity;
    return SOURCE_OK;
}
