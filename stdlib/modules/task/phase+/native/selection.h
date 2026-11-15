/** @file
 * @brief Compute the exact one-hot representation before publishing any task phase fields.
 * [ASYNC-03](../../../../../spec/language/async-and-parallelism.md#async-03)
 * requires one phase per step, independently of its application failure value.
 */
#ifndef AMA_STANDARD_TASK_PHASE_SELECTION_H
#define AMA_STANDARD_TASK_PHASE_SELECTION_H
#include "../../phase.h"
/** Prepare three predicates in ready/awaiting/completed order. Invalid choices
 * leave the caller's three-slot output unchanged; valid choices set exactly one.
 */
static inline bool AMA_NATIVE(select)(unsigned kind, bool predicates[3]) {
    if (kind != AMA_NATIVE(READY) && kind != AMA_NATIVE(AWAITING)
        && kind != AMA_NATIVE(COMPLETED)) {
        return false;
    }
    predicates[0] = kind == AMA_NATIVE(READY);
    predicates[1] = kind == AMA_NATIVE(AWAITING);
    predicates[2] = kind == AMA_NATIVE(COMPLETED);
    return true;
}
#endif
