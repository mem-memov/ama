/** @file
 * @brief Plan every participating native error namespace from current collision-checked full module identities before emission.
 * [ERROR-04](../../../../spec/implementation/diagnostics.md#error-04) requires
 * component-local nonzero provider namespaces. [NAME-05](../../../../spec/implementation/native-interface.md#name-05)
 * includes the root family and complete module key, so discovery/type-table
 * order cannot substitute for provider order. The scaffold owns planning scratch;
 * provider rows borrow its identity registry through destruction.
 */
#include "native/scaffold/internal.h"

/** Collect only native module identities, publish a complete sorted plan, then associate each model type with its exact full-key assignment. Component contract: [ERROR-04](../../../../spec/implementation/diagnostics.md#error-04), [NAME-05](../../../../spec/implementation/native-interface.md#name-05). */
bool scaffold_providers(NativeScaffold *scaffold) {
    size_t count = 0;
    for (const ScaffoldType *type = scaffold->types; type != NULL; type = type->next) {
        if (type->plan->type->module->syntax.sections.kind == MODULE_NATIVE) { count++; }
    }
    if (count > UINT16_MAX || count > SIZE_MAX / sizeof(const IdentityName *)) {
        return scaffold_error(scaffold, DIAGNOSTIC_TYPE_MISMATCH, "Native error provider namespace capacity is exhausted", NULL);
    }
    const IdentityName **names = count == 0 ? NULL : scaffold_allocate(scaffold, count * sizeof(*names));
    if (count != 0 && names == NULL) { return false; }
    size_t index = 0;
    for (const ScaffoldType *type = scaffold->types; type != NULL; type = type->next) {
        if (type->plan->type->module->syntax.sections.kind == MODULE_NATIVE) { names[index++] = type->native_prefix; }
    }
    AmaStatus status = native_status_providers_build(&scaffold->providers, names, count);
    if (status.code != AMA_OK) {
        return scaffold_error(scaffold, status.code == AMA_OUT_OF_MEMORY ? DIAGNOSTIC_OUT_OF_MEMORY : DIAGNOSTIC_TYPE_MISMATCH,
            "Cannot plan complete native error provider identities", NULL);
    }
    for (ScaffoldType *type = scaffold->types; type != NULL; type = type->next) {
        if (type->plan->type->module->syntax.sections.kind != MODULE_NATIVE) { continue; }
        status = native_status_provider_find(&scaffold->providers, type->native_prefix, &type->error_provider);
        if (status.code != AMA_OK) {
            return scaffold_error(scaffold, DIAGNOSTIC_TYPE_MISMATCH, "A native module has no exact planned error provider", type->ama_path);
        }
    }
    return true;
}
