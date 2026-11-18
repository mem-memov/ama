/** @file
 * @brief Retain prepared static operands separately from application provenance.
 * [TPL-08](../../../../spec/language/templates.md#tpl-08) preserves scoped inputs.
 * [TPL-12](../../../../spec/language/static-results.md#tpl-12) separates static outputs.
 */
#ifndef AMA_STATIC_TERM_APPLICATION_H
#define AMA_STATIC_TERM_APPLICATION_H
#include "store.h"
#include "../bindings/owner.h"

/** Supplied operands name exact nonomitted input requests, not positions guessed
 * from expected signatures. [TPL-08](../../../../spec/language/templates.md#tpl-08).
 */
typedef struct { const BindingRequest *request; const StaticTerm *value; } StaticTermOperand;
/** Source-order input record retains skips and unknown actual slots. The
 * original request owns provenance. [TPL-12](../../../../spec/language/static-results.md#tpl-12).
 */
typedef struct StaticTermInput {
    struct StaticTermInput *next;
    const BindingRequest *request;
    const StaticTerm *value;
} StaticTermInput;
/** Distinct uses survive even when their head/operand values are canonical equal.
 * Borrow the binding plan until term-owner destruction; outputs remain in syntax.
 * [TPL-08](../../../../spec/language/templates.md#tpl-08) retains application structure.
 */
typedef struct StaticTermApplication {
    struct StaticTermApplication *next;
    const BindingApplication *source;
    const StaticTermEnvironment *environment;
    const StaticTerm *head;
    StaticTermInput *first;
    StaticTermInput *last;
    size_t input_count;
} StaticTermApplication;
/** Import an accepted static request with exactly one prepared term per explicit
 * input; skipped positions have no value. No implicit inference is performed.
 * Publish only on success. [TPL-08](../../../../spec/language/templates.md#tpl-08).
 */
bool static_term_application(StaticTermStore *, const BindingPlan *, const BindingApplication *,
    const StaticTermEnvironment *, const StaticTerm *, const StaticTermOperand *, size_t,
    const StaticTermApplication **);
#endif
