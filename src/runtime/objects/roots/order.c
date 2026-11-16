/**
 * @file
 * @brief Maintain successful root publication order independently of pool allocation.
 *
 * [LIFE-05](../../../../spec/implementation/native-interface.md#life-05) requires
 * reverse successful-creation cleanup. Stable-cell links are runtime ownership
 * bookkeeping, never Ama references or native payload. They need no sequence
 * counter, allocation or payload relocation and survive replacement bank changes.
 */
#include "runtime/objects/internal.h"

/** Append one completed independent root after publication, without a fallible step.
 * Only public create/clone register roots; private construction and adopted list
 * elements never enter this chain. The world operation guard excludes mutation.
 * Component contract: [LIFE-05](../../../../spec/implementation/native-interface.md#life-05). */
void ama_object_root_append(AmaWorld *world, AmaHandle root, AmaPoolCell *cell) {
    cell->root_type_id = root.type_id;
    cell->root_previous = world->last_root;
    cell->root_next = NULL;
    if (world->last_root != NULL) { world->last_root->root_next = cell; }
    else { world->first_root = cell; }
    world->last_root = cell;
}

/** Remove a registered root in constant time before its stable cell is released.
 * Zero root_type_id identifies unregistered children/private work. Replacement
 * retaining the root slot does not call this operation, even while identity is
 * temporarily invalidated during irreversible old-value cleanup.
 * Component contract: [LIFE-05](../../../../spec/implementation/native-interface.md#life-05). */
void ama_object_root_remove(AmaWorld *world, AmaPoolCell *cell) {
    if (cell->root_type_id == 0) { return; }
    if (cell->root_previous != NULL) { cell->root_previous->root_next = cell->root_next; }
    else { world->first_root = cell->root_next; }
    if (cell->root_next != NULL) { cell->root_next->root_previous = cell->root_previous; }
    else { world->last_root = cell->root_previous; }
    cell->root_previous = cell->root_next = NULL;
    cell->root_type_id = 0;
}
