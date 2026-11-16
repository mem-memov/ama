/** @file
 * @brief Define fixed lexical cursor and reference-binding storage for live scans.
 * [FLOW-05](../../../../../spec/language/lists.md#flow-05) advances a scalar
 * boundary before entering the body and activates locals only for eligible
 * entries. [LIST-13](../../../../../spec/language/lists.md#list-13) requires
 * short acquisitions without persistent collection-buffer pointers.
 */
#ifndef AMA_PROGRAM_LIST_TRAVERSAL_H
#define AMA_PROGRAM_LIST_TRAVERSAL_H
#include "runtime/program/lists/service.h"

/** Compiler-owned binding metadata, separate from an ordinary declared handle.
 * local_value is used only for a lexical local. roles counts active installations;
 * nested reuse replaces the current origin without restoring the outer value.
 * Component contract: [FLOW-05](../../../../../spec/language/lists.md#flow-05), [LIST-13](../../../../../spec/language/lists.md#list-13). */
typedef struct AmaProgramForeachBinding {
    AmaHandle local_value;
    AmaMembership origin;
    AmaHandle installed;
    size_t roles;
    bool associated;
} AmaProgramForeachBinding;
/** Immutable offsets into one generated owner representation. value_offset names
 * either a declared reference or this binding's local_value, never foreign data.
 * Component contract: [FLOW-05](../../../../../spec/language/lists.md#flow-05), [LIST-13](../../../../../spec/language/lists.md#list-13). */
typedef struct AmaProgramForeachPlan {
    size_t field, cursor_offset, binding_offset, value_offset;
    uint64_t element_type;
    bool local;
} AmaProgramForeachPlan;
/** One lexical scan's checked collection and numerical future boundary. All
 * retained identities are values; no acquisition or entry-buffer pointer survives.
 * Component contract: [FLOW-05](../../../../../spec/language/lists.md#flow-05), [LIST-13](../../../../../spec/language/lists.md#list-13). */
typedef AmaListCursor AmaProgramForeachCursor;
/** Immutable preorder inventory. parent is an earlier foreach scope or SIZE_MAX;
 * header identifies diagnostics and repeat identifies the next selection label.
 * Component contract: [FLOW-05](../../../../../spec/language/lists.md#flow-05), [LIST-13](../../../../../spec/language/lists.md#list-13). */
typedef struct AmaProgramForeachScope {
    AmaProgramForeachPlan plan;
    size_t parent;
    uint64_t header, repeat;
} AmaProgramForeachScope;
/** Acquire/validate an empty parsed body's collection once; touch no cursor/cell. Component contract: [FLOW-05](../../../../../spec/language/lists.md#flow-05), [LIST-13](../../../../../spec/language/lists.md#list-13). */
AmaStatus ama_program_foreach_empty(AmaProgram *, AmaHandle, const AmaProgramForeachPlan *);
/** Establish boundary zero without installing a binding or acquiring a target. Component contract: [FLOW-05](../../../../../spec/language/lists.md#flow-05), [LIST-13](../../../../../spec/language/lists.md#list-13). */
AmaStatus ama_program_foreach_enter(AmaProgram *, AmaHandle, const AmaProgramForeachPlan *);
/** Advance before selection, skip only absent reference targets, and release the
 * list before returning has_value=true. Exhaustion closes this lexical role.
 * Component contract: [FLOW-05](../../../../../spec/language/lists.md#flow-05), [LIST-13](../../../../../spec/language/lists.md#list-13). */
AmaStatus ama_program_foreach_next(AmaProgram *, AmaHandle, const AmaProgramForeachPlan *, bool *has_value);
/** Discard one cursor and its binding role without touching referenced targets.
 * abandoning accepts an inactive slot; collection replacement cannot block exit.
 * Component contract: [FLOW-05](../../../../../spec/language/lists.md#flow-05), [LIST-13](../../../../../spec/language/lists.md#list-13). */
AmaStatus ama_program_foreach_leave(AmaProgram *, AmaHandle, const AmaProgramForeachPlan *, bool abandoning);
/** Select a typed binding cell; an unactivated lexical local is unavailable. Component contract: [FLOW-05](../../../../../spec/language/lists.md#flow-05), [LIST-13](../../../../../spec/language/lists.md#list-13). */
AmaStatus ama_program_foreach_value(AmaProgram *, AmaHandle, const AmaProgramForeachPlan *, AmaProgramValue *);
/** Validate exact scope/binding extents and parent preorder before context creation. Component contract: [FLOW-05](../../../../../spec/language/lists.md#flow-05), [LIST-13](../../../../../spec/language/lists.md#list-13). */
AmaStatus ama_program_foreach_schema(size_t, const AmaObjectField *, size_t,
    const AmaProgramForeachScope *, size_t);
/** Check one associated binding before target access; pure missing/busy checks
 * set refused, while invalid metadata remains technical. Plain handle copies do
 * not call this helper and never inherit an origin association.
 * Component contract: [FLOW-05](../../../../../spec/language/lists.md#flow-05), [LIST-13](../../../../../spec/language/lists.md#list-13). */
AmaStatus ama_program_foreach_guard(AmaProgram *, const AmaProgramValue *,
    const AmaProgramListService *, bool *refused);
/** Validate a staged owning-copy source's exact origin after inputs are ready. Component contract: [FLOW-05](../../../../../spec/language/lists.md#flow-05), [LIST-13](../../../../../spec/language/lists.md#list-13). */
AmaStatus ama_program_foreach_origin(AmaProgram *, AmaMembership, AmaHandle,
    const AmaProgramListService *, bool *refused);
/** Detach only when explicit assignment replaces the actual stored handle. Component contract: [FLOW-05](../../../../../spec/language/lists.md#flow-05), [LIST-13](../../../../../spec/language/lists.md#list-13). */
void ama_program_foreach_detach(AmaProgramValue *, AmaHandle);
/** Discard all installed scopes innermost first, preserving ordinary reference values. Component contract: [FLOW-05](../../../../../spec/language/lists.md#flow-05), [LIST-13](../../../../../spec/language/lists.md#list-13). */
AmaStatus ama_program_foreach_reset(AmaProgram *, AmaHandle);
/** Validate active collections outermost first. A missing current entry abandons
 * descendants and returns that loop's repeat selector; zero preserves saved body
 * dispatch. Collection/acquisition errors remain technical at the header.
 * Component contract: [FLOW-05](../../../../../spec/language/lists.md#flow-05), [LIST-13](../../../../../spec/language/lists.md#list-13). */
AmaStatus ama_program_foreach_resume(AmaProgram *, AmaHandle, uint64_t *repeat);
#endif
