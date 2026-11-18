/** @file
 * @brief Define temporary exact validation classes for complete keys and unresolved lexical owners.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) compares closed endpoints through cached paths.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) retains symbolic environments.
 */
#ifndef AMA_STATIC_CREATION_VIEW_H
#define AMA_STATIC_CREATION_VIEW_H
#include "../plan.h"

/** Complete keys share their canonical vertex. Unresolved scopes share only
 * the same exact guard owner; neither source name nor expected signature merges them.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves symbolic scope.
 */
typedef struct {
    StaticCreationVertex *vertex;
    const StaticGuardOwner *owner;
    const StaticModule *origin;
} StaticCreationViewNode;

/** All storage is borrowed from one operation's scratch arena, transferred to
 * the failed owner only when needed to retain a semantic witness.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) retains incoming provenance.
 */
typedef struct {
    StaticCreationViewNode *nodes;
    size_t count;
    bool partial_edges;
} StaticCreationView;

/** Build exact classes for all concrete vertices and required unresolved scopes.
 * No symbolic class is published as a concrete specialization/cache key.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) retains unknown scopes without merging their keys.
 */
bool static_creation_view(StaticGuardPlan *, SourceArena *, StaticCreationView *);
/** Map one registered frame to its complete-key or exact unresolved-scope class.
 * Missing metadata fails explicitly; no default index or guessed origin exists.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) preserves source scope.
 */
bool static_creation_view_path(StaticGuardPlan *, const StaticCreationView *, const StaticCreationPath *, size_t *);
#endif
