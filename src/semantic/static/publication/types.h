/** @file
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) owns original selector and residual attribution through failure reporting.
 * @brief Bind ordinary Type obligations and independently concrete static exports.
 * [TPL-08](../../../../spec/language/templates.md#tpl-08) retains the complete residual set while diagnostics select a concrete unfilled witness.
 * [TYPE-01](../../../../spec/language/declarations.md#type-01) separates consumer Types from static producers.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) requires independent producer closure.
 * [CONST-03](../../../../spec/language/declarations.md#const-03) requires complete constant namespaces.
 * [TPL-04](../../../../spec/language/templates.md#tpl-04) permits original per-field annotation equations.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) keeps metadata separate from actual expansion.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03) owns partial metadata.
 * [TPL-15](../../../../spec/language/templates.md#tpl-15) retains pure equations
 * for all original input operands before actual body validation.
 */
#ifndef AMA_STATIC_ORDINARY_TYPES_H
#define AMA_STATIC_ORDINARY_TYPES_H
#include "plan.h"

/** Private publication-owned operand cache; no runtime context is stored.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) separates facts and
 * validation; [TPL-15](../../../../spec/language/templates.md#tpl-15) keeps
 * nested expressions in their original enclosing environment.
 */
struct StaticOrdinaryOperand;
struct StaticOrdinaryProjection;

/** One original ordinary Type occurrence in an exact source environment, with
 * optional actual incoming provenance.
 * value is the consumer-local Type variable, not the supplying producer's output.
 * A null context denotes source-only metadata, never an invented actual root.
 * Every form retains an annotation source; actual uses additionally retain
 * guarded work. Selected operands retain exact supplier requests and attributed
 * graph/definition proofs. A source-only predecessor is not an ordinary context.
 * Structural export equations may refine value before supplier closure; only
 * complete supplier and graph proofs authorize publication.
 * The publication arena owns records; all pointers borrow longer-lived metadata.
 * [TYPE-01](../../../../spec/language/declarations.md#type-01) requires unique structural Type determination before storage admission.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) keeps producers out of runtime storage.
 */
typedef struct StaticOrdinaryType {
    struct StaticOrdinaryType *next;
    const StaticResultContext *context;
    const StaticTermEnvironment *environment;
    const AstTypeExpression *expression;
    const StaticTerm *value;
    StaticResultUse *source;
    const StaticResultSource *annotation;
    const StaticTermApplication *prepared;
    const StaticApplicationRecord *application;
    /** Complete original-child cache under [TPL-15](../../../../spec/language/templates.md#tpl-15).
     * The flag certifies traversal only, never Type closure or actual body
     * validation under [TPL-11](../../../../spec/language/templates.md#tpl-11).
     */
    struct StaticOrdinaryOperand *inputs;
    bool inputs_collected;
    struct StaticOrdinaryProjection *projection;
    const StaticCreationPath *annotation_path;
    const StaticModule *producer;
    const StaticSymbol *selected;
    const StaticPublication *publication;
    /** Error attribution is owned here and borrows this actual selector's source.
     * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) preserves caller context without allocating on failure.
     */
    DiagnosticContext error_context;
    /** One actual residual is a sufficient witness that a returned Template is
     * not Type; this frame does not limit or truncate attribution ancestry.
     * [TPL-08](../../../../spec/language/templates.md#tpl-08) retains every original residual in metadata.
     * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) points to the actual unfilled declaration.
     */
    DiagnosticContext residual_context;
} StaticOrdinaryType;

/** Register original ordinary annotation syntax in an exact environment
 * without inventing an actual incoming context. It owns only source metadata
 * and consumer Type obligations; no body or definition is read. This is not a
 * checked/executable use. Output stays unchanged on failure.
 * [TYPE-01](../../../../spec/language/declarations.md#type-01) limits ordinary annotation forms.
 * [TPL-11](../../../../spec/language/templates.md#tpl-11) separates source facts and guarded expansion.
 */
bool static_ordinary_source(StaticPublicationPlan *, const StaticTermEnvironment *,
    const AstTypeExpression *, StaticOrdinaryType **);
/** Generate finite ordinary annotation equations and advance attributed static
 * supplier graphs without premature closure. No ordinary root/body is created or validated; actual
 * uses still require their registered context and guarded evaluation. Changed
 * means new structural facts or required validation proofs, never allocations.
 * Failed outcome stays unchanged and partial facts remain owned.
 * [TPL-04](../../../../spec/language/templates.md#tpl-04) permits per-field omission equations.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) retains every supplier input as a publication obligation.
 */
bool static_ordinary_equations(StaticPublicationPlan *, StaticOrdinaryType *, bool *);

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
 * later runtime-call evidence. This operation requires an actual context;
 * source-only metadata must use the equation operation instead. Normal READY certifies structural work, not value
 * closure: bare unknown Type forwarding may already be READY. Final resolution
 * separately requires a concrete Type. Failures preserve the outcome receiver and leave
 * partial records owned; ordinary Pending remains a nonfailure.
 * [TYPE-01](../../../../spec/language/declarations.md#type-01) requires complete ordinary Type metadata before admission.
 */
bool static_ordinary_evaluate(StaticPublicationPlan *, StaticOrdinaryType *, StaticResultOutcome *);
/** Require a complete ordinary Type and retained source/producer publication.
 * Returned metadata is borrowed canonical identity, never runtime storage.
 * Source-only exports can resolve independently; a normal annotation needs its
 * actual incoming context for this validation operation.
 * Existing results are rechecked against current equations before reuse; failures
 * preserve output and previously published immutable descriptors.
 * [TYPE-01](../../../../spec/language/declarations.md#type-01) excludes residual Templates.
 * [CONST-03](../../../../spec/language/declarations.md#const-03) requires concrete namespaces.
 */
bool static_ordinary_resolve(StaticPublicationPlan *, StaticOrdinaryType *, const StaticTerm **);
#endif
