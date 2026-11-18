/** @file
 * @brief Represent scoped static value definitions separately from creation and equality relations.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) requires an acyclic definition graph.
 */
#ifndef AMA_STATIC_DEFINITION_MODEL_H
#define AMA_STATIC_DEFINITION_MODEL_H
#include "../model.h"

/** Original scoped nodes keep their edges after equivalent complete owner keys
 * acquire a shared representative; no source-only aliasing chooses that identity.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves scoped definitions.
 */
typedef struct StaticDefinitionNode StaticDefinitionNode;
/** Dependency occurrences retain original source sites and definition identities.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) retains dependency evidence.
 */
typedef struct StaticDefinitionEdge {
    struct StaticDefinitionEdge *next;
    StaticDefinitionNode *target;
    SourceSpan site;
} StaticDefinitionEdge;

/** A source result or receiver alias in one lexical producer environment.
 * Concrete identity additionally uses its complete normalized owner Type.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) separates definition and equality cycles.
 */
struct StaticDefinitionNode {
    StaticDefinitionNode *next;
    const StaticGuardOwner *owner;
    const StaticSymbol *symbol;
    const StaticTerm *concrete;
    StaticDefinitionNode *representative;
    size_t ordinal;
    StaticDefinitionEdge *first;
    StaticDefinitionEdge *last;
};

/** Every required definition occurrence retains its producer-creation ancestry
 * and active definition ancestry separately. Sibling reads do not form a cycle.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) guards both dependency paths.
 */
typedef struct StaticDefinitionPath {
    struct StaticDefinitionPath *next;
    const struct StaticDefinitionPath *parent;
    const StaticCreationPath *creation;
    StaticDefinitionNode *node;
    const StaticDefinitionEdge *edge;
    SourceSpan site;
} StaticDefinitionPath;

/** A concrete/scoped graph search witness uses parent links into retained
 * scratch, never recursion proportional to source-definition depth.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) retains cycle evidence.
 */
typedef struct StaticDefinitionSearch {
    StaticDefinitionNode *node;
    struct StaticDefinitionSearch *parent;
    const StaticDefinitionEdge *edge;
} StaticDefinitionSearch;
#endif
