/** @file
 * @brief Admit exact delivery modes, UTF-8 component identities and completed executable roots.
 * [CLI-01](../../../spec/implementation/launch-and-embedding.md#cli-01)
 * requires ordinary concrete executable admission; storage layout alone is not
 * execution evidence. [EMBED-06](../../../spec/implementation/c-library.md#embed-06)
 * retains complete component identity without lossy name normalization.
 */
#include <string.h>
#include "internal.h"
#include "source/unicode/unicode.h"

bool delivery_selection_validate(const CompilerFrontend *frontend, const DeliveryRequest *request, Diagnostic *error) {
    if (!frontend || !request || !error) { return false; }
    if (request->mode != DELIVERY_EXECUTABLE && request->mode != DELIVERY_SOURCE_INCLUSION) {
        *error = (Diagnostic){.code = DIAGNOSTIC_INVALID_PATH, .message = "Unknown explicit delivery output mode"};
        return false;
    }
    size_t offset;
    if (!request->component || !*request->component ||
        !utf8_validate((const unsigned char *)request->component, strlen(request->component), &offset)) {
        *error = (Diagnostic){.code = DIAGNOSTIC_INVALID_PATH, .message = "Delivery requires one nonempty complete UTF-8 component identity"};
        return false;
    }
    if (!frontend->valid || frontend->authoring || !frontend->plan.entry || !frontend->plan.entry->type ||
        !frontend->plan.entry->type->executable ||
        frontend->plan.entry->type->module->syntax.sections.kind == MODULE_STATIC) {
        *error = (Diagnostic){.code = DIAGNOSTIC_TYPE_MISMATCH, .message = "Delivery requires one checked concrete ordinary or native executable root"};
        return false;
    }
    return true;
}
