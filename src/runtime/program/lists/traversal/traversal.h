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
 */
typedef struct {
    AmaHandle local_value;
    AmaMembership origin;
    AmaHandle installed;
    size_t roles;
    bool associated;
} AmaProgramForeachBinding;
/** Immutable offsets into one generated owner representation. value_offset names
 * either a declared reference or this binding's local_value, never foreign data.
 */
typedef struct {
    size_t field, cursor_offset, binding_offset, value_offset;
    uint64_t element_type;
    bool local;
} AmaProgramForeachPlan;
/** One lexical scan's checked collection and numerical future boundary. All
 * retained identities are values; no acquisition or entry-buffer pointer survives.
 */
typedef AmaListCursor AmaProgramForeachCursor;
/** Acquire/validate an empty parsed body's collection once; touch no cursor/cell. */
AmaStatus ama_program_foreach_empty(AmaProgram *, AmaHandle, const AmaProgramForeachPlan *);
/** Establish boundary zero without installing a binding or acquiring a target. */
AmaStatus ama_program_foreach_enter(AmaProgram *, AmaHandle, const AmaProgramForeachPlan *);
/** Advance before selection, skip only absent reference targets, and release the
 * list before returning has_value=true. Exhaustion closes this lexical role.
 */
AmaStatus ama_program_foreach_next(AmaProgram *, AmaHandle, const AmaProgramForeachPlan *, bool *has_value);
/** Discard one cursor and its binding role without touching referenced targets.
 * abandoning accepts an inactive slot; collection replacement cannot block exit.
 */
AmaStatus ama_program_foreach_leave(AmaProgram *, AmaHandle, const AmaProgramForeachPlan *, bool abandoning);
/** Select a typed binding cell; an unactivated lexical local is unavailable. */
AmaStatus ama_program_foreach_value(AmaProgram *, AmaHandle, const AmaProgramForeachPlan *, AmaProgramValue *);
#endif
