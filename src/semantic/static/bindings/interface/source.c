/**
 * @file
 * @brief Freeze actual source-module interface sequences before any argument is applied.
 *
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) restricts
 * original input order to residual formals and separates explicit static outputs.
 * [CALL-06](../../../../../spec/language/calls-and-results.md#call-06) excludes
 * static declarations from runtime positions.
 * [TPL-09](../../../../../spec/language/templates.md#tpl-09) exposes a shared
 * constant namespace independently of entry execution.
 */
#include <assert.h>
#include "../internal.h"

/** Borrow exact original name bytes without normalization or allocation.
 * Contract: [TPL-12](../../../../../spec/language/static-results.md#tpl-12).
 */
static BindingName name(SourceSpan site) {
    assert(source_span_valid(site));
    return (BindingName){source_bytes(site.source) + site.start, site.end - site.start, site};
}

/** Fixed identities are selected by exact pointer, not a same-spelled foreign
 * formal or an expected signature entry. The array is borrowed during this call.
 * Contract: [TPL-12](../../../../../spec/language/static-results.md#tpl-12).
 */
static bool fixed(const StaticSymbol *symbol, const StaticSymbol *const *captures, size_t count) {
    for (size_t index = 0; index < count; index++) { if (captures[index] == symbol) { return true; } }
    return false;
}

/** Reject malformed caller metadata before preparing a snapshot. Pointer
 * membership is checked before dereferencing the supplied identity.
 * Contract: [TPL-12](../../../../../spec/language/static-results.md#tpl-12).
 */
static bool captures_valid(BindingPlan *plan, const StaticModule *module,
    const StaticSymbol *const *captures, size_t count) {
    SourceSpan site = {module->definition->source->text, 0, 0};
    if ((captures == NULL && count != 0) || count > module->parameter_count) {
        return binding_invalid(plan, site, "Invalid fixed formal identity set");
    }
    for (size_t index = 0; index < count; index++) {
        bool found = false;
        for (const StaticSymbol *symbol = module->first; symbol != NULL; symbol = symbol->next) {
            if (symbol == captures[index] && symbol->role == STATIC_PARAMETER) { found = true; break; }
        }
        if (!found) { return binding_invalid(plan, site, "Fixed formal does not belong to this actual producer"); }
        for (size_t prior = 0; prior < index; prior++) {
            if (captures[prior] == captures[index]) {
                return binding_invalid(plan, captures[index]->site, "Fixed formal identity appears more than once");
            }
        }
    }
    return true;
}

/** Use original role ordinals even though the local symbol chain groups explicit
 * binders before implicit ones. Every ordinal exists in an accepted inventory.
 * Contract: [TPL-12](../../../../../spec/language/static-results.md#tpl-12).
 */
static bool static_slots(BindingPlan *plan, BindingInterface *view,
    const StaticSymbol *const *captures, size_t capture_count) {
    for (size_t direction = BINDING_STATIC_INPUT; direction <= BINDING_STATIC_OUTPUT; direction++) {
        StaticRole role = direction == BINDING_STATIC_INPUT ? STATIC_PARAMETER : STATIC_RESULT;
        size_t count = role == STATIC_PARAMETER ? view->origin->parameter_count : view->origin->result_count;
        for (size_t ordinal = 0; ordinal < count; ordinal++) {
            const StaticSymbol *selected = static_symbol_at(view->origin, role, ordinal);
            assert(selected != NULL);
            if (role == STATIC_PARAMETER && fixed(selected, captures, capture_count)) { continue; }
            BindingSlot prototype = {.name = name(selected->name), .original_ordinal = ordinal,
                .static_symbol = selected};
            if (!binding_slot_append(plan, &view->sequences[direction], prototype)) { return false; }
        }
    }
    return true;
}

/** Only ordinary top-level declarations consume runtime positions; type formals,
 * static result definitions and their expressions have separate interfaces.
 * Contract: [CALL-06](../../../../../spec/language/calls-and-results.md#call-06).
 */
static bool runtime_slots(BindingPlan *plan, BindingSequence *sequence, AstBlock block) {
    for (const AstNode *node = block.first; node != NULL; node = node->next) {
        if (node->kind != AST_DECLARATION) { continue; }
        const AstAccess *declaration = node->as.declaration.name;
        BindingSlot prototype = {.name = name(declaration->name), .original_ordinal = sequence->count,
            .runtime_declaration = node, .reference = declaration->reference, .list = declaration->whole_list};
        if (!binding_slot_append(plan, sequence, prototype)) { return false; }
    }
    return true;
}

/** Complete all four sequences privately, then publish once. No request filling,
 * source mutation, type solving or default execution occurs here.
 * Contract: [TPL-12](../../../../../spec/language/static-results.md#tpl-12),
 * [TPL-09](../../../../../spec/language/templates.md#tpl-09).
 */
bool binding_source_interface(BindingPlan *plan, const StaticModule *module, BindingRuntimeSurface surface,
    const StaticSymbol *const *captures, size_t capture_count, BindingInterface **output) {
    if (plan->status != BINDING_OK) { return false; }
    assert(module != NULL && output != NULL);
    if (surface != BINDING_MODULE_SURFACE && surface != BINDING_CONSTANT_SURFACE) {
        return binding_invalid(plan, (SourceSpan){module->definition->source->text, 0, 0}, "Invalid runtime interface surface");
    }
    if (!captures_valid(plan, module, captures, capture_count)) { return false; }
    BindingInterface *view = binding_allocate(plan, sizeof(*view));
    if (view == NULL) { return false; }
    view->origin = module;
    view->surface = surface;
    const AstModule *syntax = &module->definition->syntax;
    if (!static_slots(plan, view, captures, capture_count)) { return false; }
    if (surface == BINDING_MODULE_SURFACE) {
        if (!runtime_slots(plan, &view->sequences[BINDING_RUNTIME_INPUT], syntax->parameters)
            || !runtime_slots(plan, &view->sequences[BINDING_RUNTIME_OUTPUT], syntax->results)) { return false; }
    } else if (!runtime_slots(plan, &view->sequences[BINDING_RUNTIME_OUTPUT], syntax->constants)) { return false; }
    binding_interface_publish(plan, view, output);
    return true;
}
