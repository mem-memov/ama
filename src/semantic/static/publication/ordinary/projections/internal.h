/** @file
 * @brief Retain selected supplier facts separately from ordinary annotation and actual-use admission.
 * [TPL-15](../../../../../../spec/language/templates.md#tpl-15) requires every
 * supplied input and a unique selected value without producer inversion.
 * [TPL-11](../../../../../../spec/language/templates.md#tpl-11) keeps lexical
 * source-only predecessors distinct from actual incoming validation scopes.
 * [ARCH-03](../../../../../../spec/implementation/compiler.md#arch-03) owns
 * partial projection records while their source/result dependencies are borrowed.
 */
#ifndef AMA_STATIC_ORDINARY_PROJECTIONS_INTERNAL_H
#define AMA_STATIC_ORDINARY_PROJECTIONS_INTERNAL_H
#include "../internal.h"

/** Publication-owned source/request cache. Context, graph and selected
 * definition borrow result-owned proofs. A null context means guarded Pending,
 * not a selected default root or a successful producer computation.
 * [ARCH-03](../../../../../../spec/implementation/compiler.md#arch-03) retains
 * failed partial records until publication/result/guard teardown in that order.
 */
typedef struct StaticOrdinaryProjection {
    const StaticResultSource *source;
    const StaticResultProducer *request;
    const StaticResultContext *context;
    StaticResultGraph *graph;
    StaticResultDefinitionUse *selected;
} StaticOrdinaryProjection;

/** Prepare one exact projection cache and guard supplier entry. All written
 * operands share the declaration's lexical owner, independently of nesting.
 * Pending succeeds without creating another supplier scope. Failed partial
 * caches remain arena-owned and never certify an ordinary body.
 * [TPL-11](../../../../../../spec/language/templates.md#tpl-11) retains the
 * original creation edge before result definitions are followed.
 */
bool static_ordinary_projection_prepare(StaticPublicationPlan *, StaticOrdinaryType *,
    const StaticResultSource *, StaticOrdinaryProjection **);
/** Advance required graphs and the exact selected definition, then contribute
 * its scalar equation only after complete supplier input/value closure. This
 * contributes facts without requiring premature closure or inventing a caller
 * context. Root selections also retain ordinary Type/marker storage restrictions.
 * [TPL-15](../../../../../../spec/language/templates.md#tpl-15) permits partial
 * intermediate constructors while requiring ordinary final declarations.
 */
bool static_ordinary_projection_equations(StaticPublicationPlan *, StaticOrdinaryType *,
    const StaticResultSource *, StaticOrdinaryProjection **);
#endif
