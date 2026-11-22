/** @file
 * @brief Join captured file and canonical unit requests with their original causes.
 * [PKG-07](../../../../../spec/project/packages.md#pkg-07) preserves exact bytes;
 * [PKG-08](../../../../../spec/project/packages.md#pkg-08) deduplicates active
 * identities independently of overlapping file memberships.
 */
#include "internal.h"
#include "source/path/logical.h"
#include <string.h>
AcquisitionClosureStatus acquisition_closure_files(AcquisitionClosure *closure, const AcquisitionUnitEntry *entries,
    const AcquisitionRoleRecord *requester, const AcquisitionVersionSource *source, AcquisitionMemberKind kind) {
    for (const AcquisitionUnitEntry *entry = entries; entry; entry = entry->next) {
        const AcquisitionMember *member = NULL;
        AcquisitionMemberStatus status = acquisition_members_add(&closure->members, entry, requester, source, kind, &member);
        if (status != ACQUISITION_MEMBER_OK) {
            return acquisition_closure_fail(closure, status == ACQUISITION_MEMBER_OUT_OF_MEMORY ? ACQUISITION_CLOSURE_OUT_OF_MEMORY
                : status == ACQUISITION_MEMBER_CONFLICT ? ACQUISITION_CLOSURE_CONFLICT : ACQUISITION_CLOSURE_INVALID,
                "Delivery membership join failed");
        }
    }
    return ACQUISITION_CLOSURE_OK;
}
AcquisitionClosureStatus acquisition_closure_request(AcquisitionClosure *closure, const char *identity, AcquisitionRole role,
    AcquisitionReasonKind reason, const AcquisitionRoleRecord *requester, const AcquisitionRoleRecord **output) {
    ModulePathShape shape;
    if (!identity || !module_path_describe((const unsigned char *)identity, strlen(identity), &shape) ||
        (shape.extent != MODULE_PATH_MODULE && (shape.extent != MODULE_PATH_VERSION || reason != ACQUISITION_SELECTED))) {
        return acquisition_closure_fail(closure, ACQUISITION_CLOSURE_INVALID, "Delivery requires a complete canonical module or selected-version identity");
    }
    AcquisitionRoleStatus status = acquisition_roles_add(&closure->roles, identity, role, reason, requester, (SourceSpan){0}, output);
    if (status != ACQUISITION_ROLE_OK) {
        return acquisition_closure_fail(closure, status == ACQUISITION_ROLE_OUT_OF_MEMORY ? ACQUISITION_CLOSURE_OUT_OF_MEMORY
            : ACQUISITION_CLOSURE_INVALID, "Delivery identity insertion failed");
    }
    return ACQUISITION_CLOSURE_OK;
}
