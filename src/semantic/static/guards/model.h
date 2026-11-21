/** @file
 * @brief Represent symbolic creation ancestry and full-key canonical creation graph identities.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) checks original template paths.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) scopes definition dependencies.
 */
#ifndef AMA_STATIC_GUARD_MODEL_H
#define AMA_STATIC_GUARD_MODEL_H
#include "../applications/plan.h"

/** A symbolic owner retains its lexical environment and either an ordinary
 * nominal shape or a compiler-only producer input map. Both may remain symbolic.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) retains unresolved environments.
 */
typedef struct StaticGuardOwner {
    struct StaticGuardOwner *next;
    const StaticTermEnvironment *environment;
    const StaticTerm *value;
    const StaticTermEnvironment *key;
} StaticGuardOwner;

/** Concrete vertices are keyed by complete normalized owner identity, never source name.
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
    /** An actual consumer requiring validation of a supplied known Type.
     * This attribution is not a constructor edge or a selected compilation root.
     * Creation ancestry follows parent; registry validation also follows this link.
     * [TPL-11](../../../../spec/language/templates.md#tpl-11) separates Type reuse from creation.
     */
    const struct StaticCreationPath *required_from;
    const StaticGuardOwner *owner;
    const StaticApplicationRecord *application;
    /** A source-only annotation predecessor records lexical creation edges,
     * never a selected root or an executable/registry use. Registry ancestry
     * stops here until an actual incoming path is checked independently.
     * [TPL-11](../../../../spec/language/templates.md#tpl-11) forbids inventing
     * roots while preserving applications originating in a lexical owner.
     */
    bool source_only;
    SourceSpan site;
    StaticCreationVertex *vertex;
    StaticCreationEdge *edge;
} StaticCreationPath;

/** One immutable provenance edge; endpoints are complete canonical owner keys.
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
    const StaticTermEnvironment *key;
    size_t ordinal;
    StaticCreationEdge *first;
    StaticCreationEdge *last;
};

/** Search entries retain an exact concrete vertex or unresolved scoped owner.
 * Path is the original incoming use; edge is present only for a concrete edge.
 * Parent links refer to retained scratch on rejection, never a recursive stack.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) reports the rejected path.
 */
typedef struct StaticCreationSearch {
    StaticCreationVertex *vertex;
    const StaticGuardOwner *owner;
    const StaticModule *origin;
    size_t ordinal;
    struct StaticCreationSearch *parent;
    StaticCreationEdge *edge;
    const StaticCreationPath *path;
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
