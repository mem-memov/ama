/** @file
 * @brief Lower ordered selection/preflight/prepare/commit/release groups over resolved receivers.
 * [CALL-08](../../../spec/language/calls-and-results.md#call-08) requires each
 * committed interface/output replacement to remain observable after later failure.
 */
#ifndef AMA_LOWER_REPLACEMENT_INTERNAL_H
#define AMA_LOWER_REPLACEMENT_INTERNAL_H
#include "lowering/control/internal.h"
/** Prepend one complete replacement group. field is mandatory only for callee installation;
 * binding is mandatory for output and optional for supplied parameters. All phases
 * share one plan-owned recipe and take technical failures to capture-before-cleanup.
 * Component contract: [CALL-08](../../../spec/language/calls-and-results.md#call-08). */
ControlNode *lower_replacement(Lowering *, const SemanticStatement *, const SemanticBinding *,
    const StorageField *, ReplacementContext, ControlNode *);
/** Stage one typed intrinsic input in its caller frame; omitted inputs use the fixed type's default, never a fake module field. Component contract: [CALL-08](../../../spec/language/calls-and-results.md#call-08). */
ControlNode *lower_method_parameter(Lowering *, const SemanticStatement *, const SemanticListField *,
    size_t, const SemanticBinding *, ControlNode *);
#endif
