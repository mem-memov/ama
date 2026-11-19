/** @file
 * @brief Own immutable static publications after exact producer and dependency closure.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) requires complete producers.
 * [TPL-08](../../../../spec/language/templates.md#tpl-08) preserves returned residual binders.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) separates publication lifetime.
 */
#ifndef AMA_STATIC_PUBLICATION_PLAN_H
#define AMA_STATIC_PUBLICATION_PLAN_H
#include "../results/plan.h"

/** Canonical selected values retain original output identities and declaration
 * order, including public outputs not selected by the caller.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) validates every output.
 */
typedef struct StaticPublishedOutput {
    struct StaticPublishedOutput *next;
    const StaticSymbol *symbol;
    const StaticTerm *value;
} StaticPublishedOutput;

/** A received name is an immutable scoped alias, never a new nominal Type.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) preserves alias equality.
 */
typedef struct StaticPublishedAlias {
    struct StaticPublishedAlias *next;
    const StaticTermEnvironment *environment;
    const StaticSymbol *symbol;
    const StaticTerm *value;
} StaticPublishedAlias;

/** A complete checked source occurrence remains distinct from its selected
 * results and from runtime instances of the resulting Type.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) preserves producer identity.
 */
typedef struct StaticPublication {
    struct StaticPublication *next;
    const StaticResultUse *use;
    /** Actual static producer scope for a result bundle; null for an ordinary
     * metadata value. Root bundles have no written use, rather than fake syntax.
     * [TPL-12](../../../../spec/language/static-results.md#tpl-12).
     */
    const StaticResultContext *context;
    const StaticTerm *value;
    StaticPublishedOutput *first_output;
    size_t output_count;
} StaticPublication;

/** Failure leaves the publication owner destructible only; borrowed semantic
 * owners preserve their own first diagnostics and original source locations.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns partial work.
 */
typedef enum { STATIC_PUBLICATION_OK, STATIC_PUBLICATION_INVALID,
    STATIC_PUBLICATION_CONFLICT, STATIC_PUBLICATION_UNRESOLVED,
    STATIC_PUBLICATION_OUT_OF_MEMORY } StaticPublicationStatus;

/** Destroy before the borrowed result, guard, application, solver and source
 * owners. Published descriptors contain no runtime storage or mutable instances.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) separates owners.
 */
typedef struct {
    SourceArena arena;
    StaticResultPlan *results;
    struct StaticOrdinaryType *first_type;
    size_t type_count;
    StaticPublication *first;
    size_t count;
    StaticPublishedAlias *first_alias;
    size_t alias_count;
    StaticPublicationStatus status;
    Diagnostic error;
} StaticPublicationPlan;

/** Borrow a healthy result owner without allocation or semantic execution.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) separates ownership.
 */
void static_publication_init(StaticPublicationPlan *, StaticResultPlan *, SourceAllocator);
/** Release only owned descriptors, not borrowed source/value graphs.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) bounds teardown.
 */
void static_publication_destroy(StaticPublicationPlan *);
/** Evaluate structural equations and existing receiver evidence to a finite
 * evidence-driven fixed point; reject unresolved producers and dependencies.
 * Publish canonical results/aliases only after closure. No final executable
 * admission or runtime effect occurs. Failed output is untouched.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) forbids incomplete publication.
 */
bool static_publication_resolve(StaticPublicationPlan *, const StaticResultContext *,
    const AstTypeExpression *, const StaticPublication **);
/** Publish all named exports of an actual static-module root after full input
 * and public-definition closure. No producer Type, synthetic call expression or
 * ordinary instance is constructed. Root and source bundle values are null;
 * output entries hold actual Type/Template metadata. Output remains unchanged
 * on refusal, including count/allocation failures; partial records stay owned.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) supports independently named export selection.
 */
bool static_publication_scope(StaticPublicationPlan *, const StaticResultContext *, const StaticPublication **);
#endif
