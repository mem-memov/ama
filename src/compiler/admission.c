/**
 * @file
 * @brief Collect immutable static source identities and gate native definitions before frontend preparation.
 *
 * [TPL-13](../../spec/language/templates.md#tpl-13) forbids native own formals
 * before accepting definitions or preparing authoring headers.
 * [ARCH-03](../../spec/implementation/compiler.md#arch-03) retains partial
 * ownership and diagnostics through the frontend's reverse teardown.
 */
#include <assert.h>
#include "internal.h"

/** Record initialization before fallible lexical collection. Both collection
 * failures and policy errors borrow persistent frontend/Project storage, never
 * a temporary related record. This phase emits or executes no native artifact.
 * Contract: [TPL-13](../../spec/language/templates.md#tpl-13),
 * [ARCH-03](../../spec/implementation/compiler.md#arch-03).
 */
bool compiler_frontend_admit(CompilerFrontend *frontend) {
    assert(frontend->project.valid && !frontend->static_initialized);
    static_inventory_init(&frontend->static_inventory, &frontend->project, frontend->allocator);
    frontend->static_initialized = true;
    if (static_inventory_collect(&frontend->static_inventory) != STATIC_OK) {
        frontend->error = frontend->static_inventory.error;
        return false;
    }
    if (!static_native_admit(&frontend->static_inventory, &frontend->native_admission)) {
        frontend->error = frontend->native_admission.error;
        return false;
    }
    return true;
}
