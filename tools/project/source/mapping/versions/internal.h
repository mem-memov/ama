/** @file
 * @brief Separate destination-demand validation from finite propagation and ownership.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * requires exact explicit placements and refuses inconsistent added-unit demands.
 */
#ifndef AMA_PROJECT_SOURCE_VERSION_PLAN_INTERNAL_H
#define AMA_PROJECT_SOURCE_VERSION_PLAN_INTERNAL_H
#include "plan.h"
/** Validate the entire borrowed graph before indexing or allocating work arrays.
 * No caller output or source state changes; canonical validation is shared with
 * the compiler rather than reconstructed from component spelling guesses.
 */
bool source_version_plan_valid(const SourceVersionUnit *, size_t, const SourceVersionFlow *, size_t);
#endif
