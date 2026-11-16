/** @file
 * @brief Share lease validation and declared-schema lookups without exposing native storage to observers.
 * [PAR-02](../../../spec/language/async-and-parallelism.md#par-02) supplies exclusive inspection authority.
 */
#ifndef AMA_DEBUGGER_INTERNAL_H
#define AMA_DEBUGGER_INTERNAL_H
#include "debugger.h"
/** Validate the live lease and idle lifecycle state before using borrowed pointers. Component contract: [PAR-02](../../../spec/language/async-and-parallelism.md#par-02). */
AmaStatus ama_debug_validate(const AmaDebugSession *);
/** Validate exact matching immutable metadata/read registrations before publishing a session. Component contract: [PAR-02](../../../spec/language/async-and-parallelism.md#par-02). */
AmaStatus ama_debug_schema(const AmaDebugSession *);
/** Find an exact registered original type name/field inventory, or leave numeric metadata unlabeled. Component contract: [PAR-02](../../../spec/language/async-and-parallelism.md#par-02). */
const AmaDebugType *ama_debug_type(const AmaDebugSession *, uint64_t);
/** Look up only the exact nonzero recorded operation; absence remains absent. Component contract: [PAR-02](../../../spec/language/async-and-parallelism.md#par-02). */
const AmaProgramLocation *ama_debug_location(const AmaDebugSession *, uint64_t);
/** Authenticate context origin and live publication without applying ordinary busy/read admission. Component contract: [PAR-02](../../../spec/language/async-and-parallelism.md#par-02). */
AmaStatus ama_debug_resolve(const AmaDebugSession *, AmaContextReference, const AmaProgramType **, AmaPoolCell **);
/** Wrap a stored context-local handle; an empty handle produces a canonical empty reference. Component contract: [PAR-02](../../../spec/language/async-and-parallelism.md#par-02). */
AmaContextReference ama_debug_reference(const AmaDebugSession *, AmaHandle);
/** Begin a synchronous visitor frame, preventing lease release until all nested frames return. Component contract: [PAR-02](../../../spec/language/async-and-parallelism.md#par-02). */
AmaStatus ama_debug_visit_begin(AmaDebugSession *);
/** End one successfully admitted visitor frame without modifying language state. Component contract: [PAR-02](../../../spec/language/async-and-parallelism.md#par-02). */
void ama_debug_visit_end(AmaDebugSession *);
#endif
