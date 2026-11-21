/**
 * @file
 * @brief Check explicit ordinary-definition provenance before dependency privileges.
 * [PKG-09](../../../../spec/project/packages.md#pkg-09) refuses src-to-tests even
 * within one version. [PKG-05](../../../../spec/project/packages.md#pkg-05)
 * permits same-version/standard targets and otherwise requires selected-unit coverage.
 */
#include "internal.h"

DependencyEdgeDecision dependency_edge_check(const DependencySelection *selection,
    const ModulePathValue *source, const ModulePathValue *target) {
    ModulePathShape from, to;
    if (source == NULL || target == NULL
        || !module_path_describe((const unsigned char *)source->text, source->length, &from)
        || !module_path_describe((const unsigned char *)target->text, target->length, &to)
        || from.root != MODULE_ROOT_PROJECT || from.extent != MODULE_PATH_MODULE || to.extent != MODULE_PATH_MODULE
        || !selection_same_version((DependencyViewEntry){.identity = source, .shape = from}, *selection->owner)) { return DEPENDENCY_EDGE_INVALID; }
    if (from.area == MODULE_PATH_SRC && to.area == MODULE_PATH_TESTS) { return DEPENDENCY_EDGE_SOURCE_TEST; }
    if (to.root == MODULE_ROOT_STANDARD
        || selection_same_version((DependencyViewEntry){.identity = source, .shape = from},
            (DependencyViewEntry){.identity = target, .shape = to})) {
        return DEPENDENCY_EDGE_ALLOWED;
    }
    return dependency_selection_covers(selection, target) ? DEPENDENCY_EDGE_ALLOWED : DEPENDENCY_EDGE_UNDECLARED;
}
