/** @file
 * @brief Promote exact declared-data module identities without hiding them behind dependency policy.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * retains full module units selected as data; [PKG-08](../../../../../spec/project/packages.md#pkg-08)
 * gives each promoted active identity exactly one expansion, even through cycles.
 */
#include <string.h>
#include "internal.h"
#include "source/path/logical.h"
AcquisitionClosureStatus acquisition_closure_declared_modules(AcquisitionClosure *closure,
    const AcquisitionRoleRecord *requester, const AcquisitionDeclaredModule *modules) {
    for (const AcquisitionDeclaredModule *module = modules; module; module = module->next) {
        if (!module->identity || !module_path_canonical((const unsigned char *)module->identity, strlen(module->identity))) {
            return acquisition_closure_fail(closure, ACQUISITION_CLOSURE_INVALID, "Declared input module lacks a complete canonical identity");
        }
        const AcquisitionRoleRecord *target = NULL;
        AcquisitionRoleStatus status = acquisition_roles_add(&closure->roles, module->identity,
            ACQUISITION_ACTIVE, ACQUISITION_DECLARED_MODULE, requester, module->origin, &target);
        if (status != ACQUISITION_ROLE_OK) {
            return acquisition_closure_fail(closure, status == ACQUISITION_ROLE_OUT_OF_MEMORY
                ? ACQUISITION_CLOSURE_OUT_OF_MEMORY : ACQUISITION_CLOSURE_INVALID,
                "Declared input module activation failed");
        }
    }
    return ACQUISITION_CLOSURE_OK;
}
