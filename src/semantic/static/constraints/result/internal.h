/** @file
 * @brief Declare iterative closed-value reconstruction over immutable canonical records.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) normalizes structural solutions.
 */
#ifndef AMA_STATIC_CONSTRAINT_RESULT_INTERNAL_H
#define AMA_STATIC_CONSTRAINT_RESULT_INTERNAL_H
#include "../internal.h"
/** Reconstruct an already checked closed graph; preserve nominal origins,
 * every original capture and independent resolved metadata, without expansion.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) preserves nominal solutions.
 */
bool static_constraint_materialize(StaticConstraintSolver *, const StaticTerm *, const StaticTerm **);
#endif
