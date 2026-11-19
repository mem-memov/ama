/** @file
 * @brief Attach guarded required-Type analysis to the loaded frontend with explicit partial-owner lifetime.
 * [COMP-02](../../spec/implementation/compiler.md#comp-02) closes Types before ordinary admission/planning.
 * [ARCH-03](../../spec/implementation/compiler.md#arch-03) retains initialization credits through failed analysis.
 */
#include <assert.h>
#include "internal.h"

/** Source/native admission already succeeded; initialize every nonfallible
 * child owner before granting the outer destruction credit. Explicit roots are
 * borrowed only during checking and must include the loaded executable entry;
 * omitting it cannot let finish bypass required entry-Type closure. Definition-only
 * analysis uses the standalone child owner. Diagnostics borrow retained sources.
 * [TPL-11](../../spec/language/templates.md#tpl-11) forbids inferred/default root selection.
 */
bool compiler_frontend_specialize(CompilerFrontend *frontend, const ProjectModule *const *roots, size_t count) {
    assert(frontend->static_initialized && frontend->static_inventory.valid && !frontend->specialization_initialized);
    compiler_specialization_init(&frontend->specializations, &frontend->static_inventory, frontend->allocator);
    frontend->specialization_initialized = true;
    bool entry_selected = false;
    if (roots) {
        for (size_t index = 0; index < count; index++) { entry_selected |= roots[index] == frontend->project.entry; }
    }
    if (!entry_selected) {
        frontend->specializations.error = (Diagnostic){.code = DIAGNOSTIC_TYPE_MISMATCH,
            .message = "Selected frontend roots must include the loaded entry module",
            .primary = {.span = {frontend->project.entry->source->text, 0, 0}}};
        frontend->error = frontend->specializations.error;
        return false;
    }
    if (!compiler_specialization_check(&frontend->specializations, roots, count)) {
        frontend->error = frontend->specializations.error;
        return false;
    }
    return true;
}
