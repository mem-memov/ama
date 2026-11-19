/** @file
 * @brief Share first-error and owned-allocation operations for the guarded closure worklist.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns failed preparation.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves source diagnostics.
 */
#ifndef AMA_STATIC_CLOSURE_INTERNAL_H
#define AMA_STATIC_CLOSURE_INTERNAL_H
#include "plan.h"
#include "../connections/internal.h"

/** Allocate zeroed arena-owned metadata only while healthy; preserve first error.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) bounds partial ownership.
 */
void *static_closure_allocate(StaticClosurePlan *, size_t);
/** Propagate borrowed semantic errors without allocating diagnostic payloads.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves first source locations.
 */
bool static_closure_dependencies(StaticClosurePlan *);
/** Record the first constant diagnostic without discarding existing partial work.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) retains original error evidence.
 */
bool static_closure_fail(StaticClosurePlan *, StaticClosureStatus, SourceSpan, const char *);
#endif
