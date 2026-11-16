/** @file
 * @brief Validate replacement operand projections separately from their executable phase groups.
 * [COMP-02](../../../../../spec/implementation/compiler.md#comp-02) permits only
 * exact resolved identities at this boundary, never emission-time name decisions.
 */
#ifndef AMA_IR_REPLACEMENT_VALIDATION_INTERNAL_H
#define AMA_IR_REPLACEMENT_VALIDATION_INTERNAL_H
#include "../internal.h"
/** Check immutable recipe operands against the exact owning semantic statement/interface. Component contract: [COMP-02](../../../../../spec/implementation/compiler.md#comp-02). */
bool plan_ir_replacement_operands(const PlannedType *, const ReplacementPlan *);
#endif
