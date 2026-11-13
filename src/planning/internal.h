/**
 * @file
 * @brief Declare typed plan allocation, storage and lifecycle/control phase contracts.
 *
 * [ARCH-03](../../spec/implementation/compiler.md#arch-03)
 * Keep plan ownership separate from semantic/project ownership and native C execution.
 */
#ifndef AMA_TYPED_PLAN_INTERNAL_H
#define AMA_TYPED_PLAN_INTERNAL_H

#include "plan.h"

/** Allocate/zero plan-owned scratch or retain an explicit allocation failure; failed partial plans remain destructible.
 */
void *plan_allocate(ProgramPlan *plan, size_t size);
/** Publish an exact borrowed source-site planning error and return false without allocating reporting storage.
 */
bool plan_fail(ProgramPlan *plan, SourceSpan site, const char *message);
/** Allocate every nominal record before linking fields/shared storage; logical slots/service masks are explicit, not C byte offsets.
 */
bool plan_storage(ProgramPlan *plan);
/** Check complete context/instance storage independently of control lowering; no allocations or execution occur. */
bool plan_validate_storage(ProgramPlan *);
/** Construct default/destruction and supported literal/copy procedures from checked type capabilities.
 */
bool plan_lifecycles(ProgramPlan *plan, PlannedType *type);
/** Allocate terminal/failure edges, lower one retained body with explicit tasks and publish its entry only after successful completion.
 */
bool plan_control(ProgramPlan *plan, PlannedType *type);
/** Allocate a unique nonzero nonwrapping selector and exact statement site; failure leaves the plan invalid.
 */
ControlNode *plan_control_node(ProgramPlan *plan, PlannedType *type,
    ControlOperation operation, const SemanticStatement *statement);
/** Find the exact resolved AST identity in the borrowed semantic statement sequence; no syntax reparsing or guessed operands.
 */
const SemanticStatement *plan_statement(const PlannedType *type, const AstNode *syntax);

#endif
