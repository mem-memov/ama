/**
 * @file
 * @brief Reject native own-formal definitions before any specialization-dependent native preparation.
 *
 * [TPL-13](../../../../spec/language/templates.md#tpl-13) uses the complete
 * own parameter set from TPL-02/TPL-12, including unused/result-only inputs.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) reports both
 * original sites without diagnostic allocation or retaining temporary storage.
 */
#include <assert.h>
#include "admission.h"

/** Original ordinal zero identifies the earliest genuine own introduction,
 * independently of the explicit/implicit grouping in the symbol owner chain.
 * Contract: [TPL-13](../../../../spec/language/templates.md#tpl-13).
 */
static const StaticSymbol *first_parameter(const StaticModule *module) {
    for (const StaticSymbol *symbol = module->first; symbol != NULL; symbol = symbol->next) {
        if (symbol->role == STATIC_PARAMETER && symbol->ordinal == 0) { return symbol; }
    }
    assert(false);
    return NULL;
}

/** A finite read-only inventory scan needs no inference or arena. A returned
 * constructor's residuals/alias metadata cannot change module.parameter_count.
 * Preserve result-owned diagnostic links until the caller finishes rendering.
 * Contract: [TPL-13](../../../../spec/language/templates.md#tpl-13),
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04).
 */
bool static_native_admit(const StaticInventory *inventory, StaticNativeAdmission *output) {
    assert(inventory != NULL && inventory->valid && output != NULL);
    for (const StaticModule *module = inventory->first; module != NULL; module = module->next) {
        if (module->definition->syntax.sections.kind != MODULE_NATIVE || module->parameter_count == 0) { continue; }
        const StaticSymbol *parameter = first_parameter(module);
        const LayoutLine *marker = module->definition->syntax.sections.implementation.marker;
        assert(marker != NULL);
        *output = (StaticNativeAdmission){
            .error = {.code = DIAGNOSTIC_TYPE_MISMATCH,
                .message = "A native @C module cannot declare own static parameters",
                .primary = {.span = marker->code}, .related_count = 1},
            .parameter = {.location = {.span = parameter->site},
                .role = "own static parameter introduction", .name = module->definition->source->logical_path}};
        output->error.related = &output->parameter;
        return false;
    }
    return true;
}
