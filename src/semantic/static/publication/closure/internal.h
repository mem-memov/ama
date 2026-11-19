/** @file
 * @brief Define exact selected-source and whole-graph closure work, independent of definition-cycle checking.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) permits finite validation groups with complete producers.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns attempt-local queue storage.
 */
#ifndef AMA_STATIC_PUBLICATION_CLOSURE_INTERNAL_H
#define AMA_STATIC_PUBLICATION_CLOSURE_INTERNAL_H
#include "../internal.h"

/** Distinct proof domains are never inferred from a nonexistent producer value.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves exact scoped metadata and its validation obligations.
 */
typedef enum { STATIC_CLOSURE_SOURCE, STATIC_CLOSURE_DEFINITION, STATIC_CLOSURE_GRAPH } StaticPublicationWorkKind;
/** One exact retained handle, tagged before any payload is followed.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12).
 */
typedef struct {
    StaticPublicationWorkKind kind;
    union {
        const StaticResultUse *source;
        const StaticResultDefinitionUse *definition;
        const StaticResultGraph *graph;
    } as;
} StaticPublicationItem;
/** Visited identity is a domain plus actual registered object, not source text.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) closes finite validation cycles.
 */
typedef struct StaticPublicationWorkNode {
    struct StaticPublicationWorkNode *next;
    StaticPublicationItem item;
} StaticPublicationWorkNode;
/** Queue storage expires after one exact closure query, including any refusal.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03).
 */
typedef struct {
    StaticPublicationPlan *plan;
    SourceArena arena;
    StaticPublicationWorkNode *first;
    StaticPublicationWorkNode *last;
    bool closed;
} StaticPublicationWork;

/** Validate and enqueue one registered proof once; output work remains owned.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12).
 */
bool static_publication_enqueue(StaticPublicationWork *, StaticPublicationItem);
/** Visit one tagged proof and enqueue its actual dependencies, without evaluating
 * unrelated roots or manufacturing definition-read edges.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12).
 */
bool static_publication_visit(StaticPublicationWork *, StaticPublicationItem);
/** Query the exact reachable metadata closure of a source or root graph.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12).
 */
bool static_publication_dependencies_closed(StaticPublicationPlan *, StaticPublicationItem, bool *);
#endif
