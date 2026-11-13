/** @file
 * @brief Construct fixed lexical storage from fully resolved control and binding identities.
 * [RT-02](../../../../spec/implementation/runtime.md#rt-02) bounds this storage
 * by lexical sites rather than runtime iteration count or same-type call depth.
 */
#ifndef AMA_PLANNING_FRAMES_INTERNAL_H
#define AMA_PLANNING_FRAMES_INTERNAL_H
#include "planning/internal.h"
/** Build cells then frame inventories, patching parents only after every lexical slot exists. */
bool plan_storage_frames(ProgramPlan *, PlannedType *);
/** Return the complete general logical contents required by this resolved lexical operation. */
unsigned plan_storage_frame_contents(const SemanticStatement *);
/** Borrow the exact frame for a resolved statement, or null for a non-frame statement. */
StorageFrame *plan_storage_frame(const PlannedType *, const SemanticStatement *);
/** Reserve one cell for each introduced lexical reference; reused bindings retain the same cell or permanent field. */
bool plan_storage_cells(ProgramPlan *, PlannedType *);
#endif
