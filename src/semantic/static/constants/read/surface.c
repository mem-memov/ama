/** @file
 * @brief Snapshot the unique own constant surface of an independently determined concrete Type.
 * [CONST-03](../../../../../spec/language/declarations.md#const-03) excludes inherited fields/results.
 * [CONST-01](../../../../../spec/language/declarations.md#const-01) forbids reference storage.
 * [NAME-04](../../../../../spec/language/declarations.md#name-04) rejects ambiguous base names.
 */
#include <stdint.h>
#include "../internal.h"
#include "../../internal.h"

/** Duplicate own base names are errors, never a choice of the first matching
 * declaration. Reference constants are invalid even when a read skips them.
 * [CONST-03](../../../../../spec/language/declarations.md#const-03) preserves exact selection.
 */
static bool declarations(StaticConstantPlan *plan, const StaticModule *module) {
    for (const AstNode *node = module->definition->syntax.constants.first; node; node = node->next) {
        if (node->kind != AST_DECLARATION) { continue; }
        if (node->as.declaration.name->reference) {
            return static_constant_fail(plan, STATIC_CONSTANT_CONFLICT, node->as.declaration.name->span,
                "Shared namespace contains reference or reference-list storage");
        }
        for (const AstNode *prior = module->definition->syntax.constants.first; prior != node; prior = prior->next) {
            if (prior->kind == AST_DECLARATION &&
                static_same_name(prior->as.declaration.name->name, node->as.declaration.name->name)) {
                return static_constant_fail(plan, STATIC_CONSTANT_CONFLICT, node->as.declaration.name->span,
                    "Shared namespace declares the same base name more than once");
            }
        }
    }
    return true;
}

/** The closed capture map filters all fixed static inputs without modifying
 * original formal/result order. Runtime selection sees only the constant surface.
 * [CONST-03](../../../../../spec/language/declarations.md#const-03) fixes namespace identity.
 */
bool static_constant_surface(StaticConstantPlan *plan, const StaticTerm *type, BindingInterface **output) {
    if (type->kind != STATIC_TERM_NOMINAL || type->category != STATIC_CATEGORY_TYPE) {
        return static_constant_fail(plan, STATIC_CONSTANT_CONFLICT, type->site, "Constant namespace must be a concrete Type");
    }
    if (!declarations(plan, type->as.nominal.origin)) { return false; }
    size_t count = type->as.nominal.capture_count;
    if (count > SIZE_MAX / sizeof(const StaticSymbol *)) {
        return static_constant_fail(plan, STATIC_CONSTANT_OUT_OF_MEMORY, type->site, "Constant namespace capture storage exceeds size limits");
    }
    const StaticSymbol **fixed = count ? static_constant_allocate(plan, count * sizeof(*fixed)) : NULL;
    if (count && fixed == NULL) { return false; }
    size_t index = 0;
    for (const StaticTermCapture *capture = type->as.nominal.first_capture; capture; capture = capture->next) {
        fixed[index++] = capture->formal;
    }
    if (!binding_source_interface(plan->publications->results->guards->applications->bindings,
        type->as.nominal.origin, BINDING_CONSTANT_SURFACE, fixed, count, output)) {
        (void)static_constant_dependencies(plan); return false;
    }
    return true;
}
