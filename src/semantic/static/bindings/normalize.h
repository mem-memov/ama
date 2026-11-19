/**
 * @file
 * @brief Normalize source requests against independently selected actual interface snapshots.
 *
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) requires named
 * static requests against full actual metadata and pending unknown origins.
 * [CALL-06](../../../../spec/language/calls-and-results.md#call-06) preserves
 * ordinary positional skips and direction order.
 * [CALL-09](../../../../spec/language/calls-and-results.md#call-09) keeps selected
 * operations separate from ordinary argument positions.
 */
#ifndef AMA_BINDING_SEQUENCE_NORMALIZE_H
#define AMA_BINDING_SEQUENCE_NORMALIZE_H
#include "owner.h"

/** Normalize one structural expression without descending into child expressions.
 * Caller independently selects the actual producer/captures; the interface must
 * belong to this plan, or be null for pending origin. Syntax/source owners outlive
 * the plan. Publish output only after all local requests pass; do not execute,
 * infer closure or substitute type terms. Nested owners normalize separately.
 * Contract: [TPL-12](../../../../spec/language/static-results.md#tpl-12).
 */
bool binding_normalize_static(BindingPlan *, const BindingInterface *,
    const AstTypeExpression *, BindingApplication **);
/** Normalize one ordinary AST_CALL argument chain, excluding selected-operation
 * actions. Caller supplies its actual runtime surface or null pending origin.
 * Retain original syntax and parser call-mode constraints; descendants normalize
 * independently. Output remains untouched on failure; no target is acquired.
 * Contract: [CALL-06](../../../../spec/language/calls-and-results.md#call-06),
 * [CALL-09](../../../../spec/language/calls-and-results.md#call-09).
 */
bool binding_normalize_runtime(BindingPlan *, const BindingInterface *, const AstNode *, BindingApplication **);
#endif
