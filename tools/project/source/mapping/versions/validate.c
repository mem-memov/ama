/** @file
 * @brief Validate exact ordinary identities, unique originals and bounded demand edges.
 * [PKG-01](../../../../../spec/project/packages.md#pkg-01) supplies canonical
 * version boundaries; [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * gives each original only one explicit or derived destination.
 */
#include "internal.h"
#include "source/path/logical.h"
#include <string.h>
/** Delegate endpoint meaning to the common bounded canonical path recognizer. */
static bool endpoint(const char *identity, ModulePathExtent extent) {
    ModulePathShape shape;
    return identity && module_path_describe((const unsigned char *)identity,
        strlen(identity), &shape) && shape.root == MODULE_ROOT_PROJECT && shape.extent == extent;
}
bool source_version_plan_valid(const SourceVersionUnit *units, size_t count,
    const SourceVersionFlow *flows, size_t flow_count) {
    if ((count && !units) || (flow_count && !flows)) { return false; }
    for (size_t i = 0; i < count; i++) {
        if (!units[i].original || !endpoint(units[i].original->logical_path, MODULE_PATH_MODULE) ||
            (units[i].fixed_version && !endpoint(units[i].fixed_version, MODULE_PATH_VERSION))) { return false; }
        for (size_t j = 0; j < i; j++) {
            if (units[i].original == units[j].original ||
                !strcmp(units[i].original->logical_path, units[j].original->logical_path)) { return false; }
        }
    }
    for (size_t i = 0; i < flow_count; i++) {
        if (flows[i].requester >= count || flows[i].target >= count) { return false; }
    }
    return true;
}
