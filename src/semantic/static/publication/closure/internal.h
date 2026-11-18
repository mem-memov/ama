/** @file
 * @brief Describe a finite selected-source closure worklist distinct from definition-cycle checks.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves receiver equality cycles and full producers.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) bounds attempt scratch.
 */
#ifndef AMA_STATIC_PUBLICATION_CLOSURE_INTERNAL_H
#define AMA_STATIC_PUBLICATION_CLOSURE_INTERNAL_H
#include "../internal.h"

/** One actual source occurrence is checked once per closure attempt, including
 * when independent receiver relationships form a permitted equality cycle.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) separates equality and definitions.
 */
typedef struct StaticPublicationWorkNode {
    struct StaticPublicationWorkNode *next;
    const StaticResultUse *use;
} StaticPublicationWorkNode;

/** Queue membership is exact handle identity; unrelated roots never enter it.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) closes required producers.
 */
typedef struct {
    StaticPublicationPlan *plan;
    SourceArena arena;
    StaticPublicationWorkNode *first;
    StaticPublicationWorkNode *last;
    bool closed;
} StaticPublicationWork;

/** Queue a checked source occurrence once, preserving finite receiver cycles.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) retains actual scope.
 */
bool static_publication_enqueue(StaticPublicationWork *, const StaticResultUse *);
/** Include a guarded definition's checked RHS proof, including cache witnesses.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) forbids cached incomplete producers.
 */
bool static_publication_definition(StaticPublicationWork *, const StaticResultDefinitionUse *);
/** Evaluate original existing receiver definitions as independent roots and
 * queue their checked source dependencies, without adding definition edges.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) separates constraints.
 */
bool static_publication_evidence(StaticPublicationWork *, const StaticResultUse *);
#endif
