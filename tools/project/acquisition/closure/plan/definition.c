/** @file
 * @brief Distinguish structural delivery activation from an active Ama definition.
 * [PKG-07](../../../../../spec/project/packages.md#pkg-07) retains complete units
 * even when their main is an inactive draft;
 * [SYN-04](../../../../../spec/language/source-and-names.md#syn-04) supplies no
 * invented semantic body; [PKG-08](../../../../../spec/project/packages.md#pkg-08)
 * applies required executable tests to actual active source definitions.
 */
#include <string.h>
#include "internal.h"
AcquisitionClosureStatus acquisition_closure_definition(AcquisitionClosure *closure, const AcquisitionRoleRecord *record,
    const AcquisitionExpansion *fact, const DeliverySourceDefinition **output) {
    const DeliverySourceDefinition *definition = fact->definitions;
    while (definition && strcmp(definition->identity, record->identity)) { definition = definition->next; }
    if (fact->draft) {
        if (definition || fact->test_directory || fact->test_files) {
            return acquisition_closure_fail(closure, ACQUISITION_CLOSURE_INVALID,
                "Draft main contradicts checked definition or automatic test association");
        }
        *output = NULL; return ACQUISITION_CLOSURE_OK;
    }
    if (!definition) {
        return acquisition_closure_fail(closure, ACQUISITION_CLOSURE_INVALID, "Selected module lacks its checked source definition");
    }
    *output = definition; return ACQUISITION_CLOSURE_OK;
}
