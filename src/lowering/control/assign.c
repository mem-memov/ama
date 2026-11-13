/**
 * @file
 * @brief Retain receiver/source selection and every replacement phase for all assignments.
 *
 * [COPY-06](../../../spec/language/values-and-references.md#copy-06)
 * Spelling-based no-op classification cannot erase static capability or runtime identity/access checks.
 */
#include "lowering/replacement/internal.h"

ControlNode *lower_assignment(Lowering *lowering, const SemanticStatement *statement, ControlNode *after) {
    return lower_replacement(lowering, statement, NULL, NULL, REPLACE_ASSIGNMENT, after);
}
