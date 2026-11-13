/** @file
 * @brief Recognize the fixed size bridge by complete checked standard-module identity before declarations or skeleton decisions.
 * [PROTO-03](../../../../spec/implementation/native-interface.md#proto-03)
 * applies only to native ..integer. Canonical spelling includes its root family;
 * neither a project basename nor an arbitrary payload layout grants this contract.
 */
#include <string.h>
#include "native/scaffold/internal.h"

/** Compare the entire canonical root-aware identity of a validated native module; no suffix, filesystem directory or helper spelling is used as evidence. */
bool scaffold_size_bridge(const ScaffoldType *type) {
    return type->plan->type->module->syntax.sections.kind == MODULE_NATIVE
        && strcmp(type->plan->type->module->source->logical_path, "..integer") == 0;
}
