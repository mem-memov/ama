/** @file
 * @brief Preserve resolved dependency targets and original causes in the delivery queue.
 * [PKG-08](../../../../../spec/project/packages.md#pkg-08) forbids text-guessing
 * imports; [DEP-03](../../../../../spec/implementation/compiler.md#dep-03) already
 * resolves source-scoped ordinary/static/constant references with original spans.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * permits retaining external bindings while selecting only copied units.
 */
#include "expand.h"
#include <string.h>
AcquisitionRoleStatus acquisition_dependencies_expand(AcquisitionRoles *roles,
    const AcquisitionRoleRecord *requester, const DeliverySourceDefinition *definition,
    AcquisitionDependencySelection select, void *context) {
    if (!roles || !requester || !definition || !definition->identity) { return ACQUISITION_ROLE_INVALID; }
    bool known_role = false;
    for (const AcquisitionRoleRecord *item = roles->records; item; item = item->next) {
        if (item == requester) { known_role = true; break; }
    }
    if (!known_role || requester->role != ACQUISITION_ACTIVE ||
        strcmp(requester->identity, definition->identity)) { return ACQUISITION_ROLE_INVALID; }
    for (const DeliverySourceDependency *edge = definition->dependencies; edge; edge = edge->next) {
        if (select && !select(context, definition, edge)) { continue; }
        const AcquisitionRoleRecord *target = NULL;
        AcquisitionRoleStatus status = acquisition_roles_add(roles, edge->target,
            ACQUISITION_ACTIVE, ACQUISITION_DEPENDENCY, requester, edge->origin, &target);
        if (status != ACQUISITION_ROLE_OK) { return status; }
    }
    return ACQUISITION_ROLE_OK;
}
