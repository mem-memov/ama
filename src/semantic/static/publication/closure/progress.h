/** @file
 * @brief Describe finite evidence that can justify another static metadata refinement attempt.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) forbids allocation-driven progress or pass budgets.
 */
#ifndef AMA_STATIC_PUBLICATION_PROGRESS_H
#define AMA_STATIC_PUBLICATION_PROGRESS_H
#include "internal.h"

/** Facts and required/checked proof records are finite under guarded expansion.
 * Canonical term allocation and work retry counts are intentionally absent.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12).
 */
typedef struct {
    size_t equations, metadata, requirements, contexts, graphs, caches, complete_graphs;
} StaticPublicationProgress;
/** Snapshot actual knowledge and independently completed graph proof witnesses.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves exact scoped metadata and its validation obligations.
 */
StaticPublicationProgress static_publication_progress(const StaticPublicationPlan *);
/** Exact named-field comparison, never struct padding or a probabilistic hash.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves exact scoped metadata and its validation obligations.
 */
bool static_publication_same_progress(StaticPublicationProgress, StaticPublicationProgress);
#endif
