/** @file
 * @brief Represent symbolic creation ancestry and full-key canonical creation graph identities.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) checks original template paths.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) scopes definition dependencies.
 */
#ifndef AMA_STATIC_GUARD_MODEL_H
#define AMA_STATIC_GUARD_MODEL_H
#include "../applications/plan.h"

/** A symbolic owner retains its lexical environment and fully bound nominal
 * shape, whose captured variables need not yet have concrete solutions.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) retains unresolved environments.
 */
typedef struct StaticGuardOwner {
    struct StaticGuardOwner *next;
    const StaticTermEnvironment *environment;
    const StaticTerm *value;
} StaticGuardOwner;

/** Concrete vertices are keyed by complete nominal identity, never source name.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) preserves distinct specializations.
 */
typedef struct StaticCreationVertex StaticCreationVertex;
/** Retain an original creation occurrence even when both endpoint keys reuse
 * existing concrete vertices.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) preserves cache incoming edges.
 */
typedef struct StaticCreationEdge StaticCreationEdge;

/** Immutable occurrence ancestry. All written children belong to the lexical
 * caller, so syntactic argument nesting does not manufacture creation edges.
 * Canonical cache views may be attached later without changing source ancestry.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) preserves incoming provenance.
 */
typedef struct StaticCreationPath {
    struct StaticCreationPath *next;
    const struct StaticCreationPath *parent;
    const StaticGuardOwner *owner;
    const StaticApplicationRecord *application;
    SourceSpan site;
    StaticCreationVertex *vertex;
    StaticCreationEdge *edge;
} StaticCreationPath;

/** One immutable provenance edge; endpoints are complete canonical Types.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) retains all use locations.
 */
struct StaticCreationEdge {
    StaticCreationEdge *next;
    StaticCreationVertex *target;
    const StaticCreationPath *path;
};

/** Full keys distinguish source-equal specializations; local dense ordinals are
 * only visited-array positions, never semantic identity or parameter order.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) forbids source-name graph merging.
 */
struct StaticCreationVertex {
    StaticCreationVertex *next;
    const StaticTerm *value;
    size_t ordinal;
    StaticCreationEdge *first;
    StaticCreationEdge *last;
};

/** Search entries retain the exact cached-edge witness; parent links refer to
 * the same retained scratch array only on rejection, not to a recursive stack.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) reports the rejected path.
 */
typedef struct StaticCreationSearch {
    StaticCreationVertex *vertex;
    struct StaticCreationSearch *parent;
    StaticCreationEdge *edge;
} StaticCreationSearch;

/** Closure concerns registered path keys only; unknown application requests
 * and executable/static-output admission remain separate obligations.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) preserves delayed inputs.
 */
typedef enum { STATIC_GUARD_CLOSED, STATIC_GUARD_PENDING } StaticGuardClosure;

/** No edge is produced by forwarding partial metadata or reusing a supplied
 * Type. Unknown actual origins remain explicit rather than selecting a source.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) distinguishes application from reuse.
 */
typedef enum { STATIC_CREATION_ENTERED, STATIC_CREATION_METADATA,
    STATIC_CREATION_REUSE, STATIC_CREATION_PENDING } StaticCreationOutcome;
#endif
