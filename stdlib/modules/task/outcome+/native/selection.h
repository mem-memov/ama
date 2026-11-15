/** @file
 * @brief Define the complete cooperative terminal transition without interpreting callback arrival as a commit.
 * [ASYNC-08](../../../../../spec/language/async-and-parallelism.md#async-08)
 * makes a committed outcome absorbing; cancellation observed at the step boundary
 * wins only while completion is still uncommitted.
 */
#ifndef AMA_TASK_OUTCOME_SELECTION_H
#define AMA_TASK_OUTCOME_SELECTION_H
#include <stdbool.h>
#include <stddef.h>
/** Validate first, then select Pending=0, Cancelled=1 or Completed=2 without changing a failed output. */
static inline bool AMA_NATIVE(select)(unsigned current, size_t cancel, size_t complete, unsigned *selected) {
    if (current > 2 || cancel > 1 || complete > 1 || selected == NULL) { return false; }
    *selected = current != 0 ? current : cancel != 0 ? 1 : complete != 0 ? 2 : 0;
    return true;
}
#endif
