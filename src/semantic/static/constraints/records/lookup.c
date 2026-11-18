/** @file
 * @brief Grow checked ordinal lookup arrays while preserving stable variable records.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns retained arrays.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) preserves variable identities.
 */
#include <stdint.h>
#include <string.h>
#include "../internal.h"

/** Capacity doubles until sufficient, capped by the exact addressable pointer
 * count. All old arrays remain arena-owned; their total size is linear in the
 * greatest requested ordinal. No binding or value identity changes.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) bounds partial growth.
 */
bool static_constraint_lookup_capacity(StaticConstraintSolver *solver, size_t needed) {
    if (needed <= solver->variable_capacity) { return true; }
    size_t maximum = SIZE_MAX / sizeof(*solver->variables);
    if (needed > maximum) {
        return static_constraint_failure(solver, STATIC_CONSTRAINT_OUT_OF_MEMORY, (SourceSpan){0},
            "Static variable lookup exceeds representable storage", NULL, NULL);
    }
    size_t capacity = solver->variable_capacity != 0 ? solver->variable_capacity : 1;
    while (capacity < needed) { capacity = capacity > maximum / 2 ? maximum : capacity * 2; }
    StaticConstraintVariable **variables = static_constraint_allocate(solver, capacity * sizeof(*variables));
    if (variables == NULL) { return false; }
    for (size_t index = 0; index < capacity; index++) { variables[index] = NULL; }
    if (solver->variable_capacity != 0) {
        memcpy(variables, solver->variables, solver->variable_capacity * sizeof(*variables));
    }
    solver->variables = variables;
    solver->variable_capacity = capacity;
    return true;
}
