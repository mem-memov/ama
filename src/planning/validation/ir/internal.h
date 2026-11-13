/** @file
 * @brief Share exact ownership-membership checks between completed typed-IR validators.
 * [COMP-02](../../../../spec/implementation/compiler.md#comp-02) requires resolved
 * operands and explicit edges before a plan can be consumed by C emission.
 */
#ifndef AMA_PLAN_IR_VALIDATION_INTERNAL_H
#define AMA_PLAN_IR_VALIDATION_INTERNAL_H
#include "planning/internal.h"
/** Test exact planned nominal identity without dereferencing an unrecognized pointer. */
bool plan_ir_type(const ProgramPlan *, const PlannedType *);
/** Test exact field ownership within an already recognized nominal record. */
bool plan_ir_field(const PlannedType *, const StorageField *);
/** Check storage identities and semantic-owner correspondence without allocating. */
bool plan_ir_storage(ProgramPlan *, const PlannedType *);
/** Check fixed lexical slots, shared binding origins and exact parent/frame identities. */
bool plan_ir_frames(ProgramPlan *, const PlannedType *);
/** Check lifecycle instruction operands, endpoints and failure graph membership. */
bool plan_ir_lifecycle(ProgramPlan *, const PlannedType *);
/** Verify CALL-08 selection/preflight/install/output order independently of runtime execution. */
bool plan_ir_invocations(ProgramPlan *, const PlannedType *);
/** Verify complete replacement recipes, exact phase groups and source-lifetime handoffs. */
bool plan_ir_replacements(ProgramPlan *, const PlannedType *);
/** Check control selectors, local/callee edges, resolved operands and source maps. */
bool plan_ir_control(ProgramPlan *, const PlannedType *);
#endif
