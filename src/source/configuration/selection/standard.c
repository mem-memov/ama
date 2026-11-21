/**
 * @file
 * @brief Check authored standard dependencies without accepting a consumer package table.
 * [PKG-05](../../../../spec/project/packages.md#pkg-05) isolates standard
 * definitions while preserving caller-supplied types outside this authored-edge
 * operation. [SRC-01](../../../../spec/language/source-and-names.md#src-01)
 * distinguishes namespace kinds and complete endpoints rather than component words.
 */
#include "selection.h"

DependencyEdgeDecision dependency_standard_edge_check(const ModulePathValue *source,
    const ModulePathValue *target) {
    ModulePathShape from, to;
    if (source == NULL || target == NULL
        || !module_path_describe((const unsigned char *)source->text, source->length, &from)
        || !module_path_describe((const unsigned char *)target->text, target->length, &to)
        || from.root != MODULE_ROOT_STANDARD || from.extent != MODULE_PATH_MODULE
        || to.extent != MODULE_PATH_MODULE) { return DEPENDENCY_EDGE_INVALID; }
    return to.root == MODULE_ROOT_STANDARD ? DEPENDENCY_EDGE_ALLOWED : DEPENDENCY_EDGE_STANDARD_ORDINARY;
}
