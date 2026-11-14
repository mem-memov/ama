/** @file
 * @brief Borrow bounded lexical scan slots for one checked operation only.
 * [RT-02](../../../../../spec/implementation/runtime.md#rt-02) stores cursor and
 * binding state in stable owner slots; borrowed pointers never enter saved state.
 */
#ifndef AMA_PROGRAM_FOREACH_INTERNAL_H
#define AMA_PROGRAM_FOREACH_INTERNAL_H
#include "traversal.h"
#include "runtime/program/calls/call.h"
/** Ephemeral typed views of validated generated offsets, not saved control data. */
typedef struct {
    AmaProgramForeachCursor *cursor;
    AmaProgramForeachBinding *binding;
    AmaHandle *value;
} AmaForeachSlots;
/** Resolve owner/field/type and validate every size/alignment before publishing views. */
AmaStatus ama_foreach_slots(AmaProgram *, AmaHandle, const AmaProgramForeachPlan *, AmaForeachSlots *);
/** Resolve an active cursor and require the exact saved collection incarnation. */
AmaStatus ama_foreach_acquire(AmaProgram *, AmaHandle, const AmaProgramForeachPlan *,
    AmaForeachSlots *, AmaProgramListService *);
#endif
