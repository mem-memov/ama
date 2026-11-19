/** @file
 * @brief Bind ordinary Type obligations and independently concrete static exports.
 * [TYPE-01](../../../../spec/language/declarations.md#type-01) separates consumer Types from static producers.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) requires independent producer closure.
 * [CONST-03](../../../../spec/language/declarations.md#const-03) requires complete constant namespaces.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns partial metadata.
 */
#ifndef AMA_STATIC_ORDINARY_TYPES_H
#define AMA_STATIC_ORDINARY_TYPES_H
#include "plan.h"

/** One original ordinary Type occurrence under an actual incoming context.
 * value is the consumer-local Type variable, not the supplying producer's output.
 * Normal forms retain a result source. Export forms instead retain the exact
 * static module/public symbol and its independent root/publication proof.
 * No equation connects an export to value before independent closure succeeds.
 * The publication arena owns records; all pointers borrow longer-lived metadata.
 * [TYPE-01](../../../../spec/language/declarations.md#type-01) forbids contextual factory inference.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) keeps producers out of runtime storage.
 */
typedef struct StaticOrdinaryType {
    struct StaticOrdinaryType *next;
    const StaticResultContext *context;
    const AstTypeExpression *expression;
    const StaticTerm *value;
    StaticResultUse *source;
    const StaticModule *producer;
    const StaticSymbol *selected;
    const StaticResultContext *producer_context;
    const StaticPublication *publication;
} StaticOrdinaryType;

/** Register exact syntax/context and a Type obligation without using ordinary
 * receiver or initializer evidence to determine a static supplier. Normal field
 * inference retains its ordinary policy. Foreign/copied syntax and wrong module
 * kinds/selectors fail before publication; outputs remain untouched on failure.
 * [TYPE-01](../../../../spec/language/declarations.md#type-01) defines ordinary forms.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) requires one named export.
 */
bool static_ordinary_type(StaticPublicationPlan *, const StaticResultContext *,
    const AstTypeExpression *, StaticOrdinaryType **);
/** Advance normal structural inference, or independently close a static supplier
 * before adding the consumer equality. A static export cannot remain open for
 * later runtime-call evidence. Normal READY certifies structural work, not value
 * closure: bare unknown Type forwarding may already be READY. Final resolution
 * separately requires a concrete Type. Failures preserve the outcome receiver and leave
 * partial records owned; ordinary Pending remains a nonfailure.
 * [TYPE-01](../../../../spec/language/declarations.md#type-01) forbids reverse supplier inference.
 */
bool static_ordinary_evaluate(StaticPublicationPlan *, StaticOrdinaryType *, StaticResultOutcome *);
/** Require a complete ordinary Type and retained source/producer publication.
 * Returned metadata is borrowed canonical identity, never runtime storage.
 * Existing results are rechecked against current equations before reuse; failures
 * preserve output and previously published immutable descriptors.
 * [TYPE-01](../../../../spec/language/declarations.md#type-01) excludes residual Templates.
 * [CONST-03](../../../../spec/language/declarations.md#const-03) requires concrete namespaces.
 */
bool static_ordinary_resolve(StaticPublicationPlan *, StaticOrdinaryType *, const StaticTerm **);
#endif
