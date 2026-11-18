/**
 * @file
 * @brief Keep immutable operand-cache records and collection private to preparation.
 *
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) preserves each
 * original lexical child. [TPL-11](../../../../../spec/language/templates.md#tpl-11)
 * requires actual uses to retain their expansion checks after metadata sharing.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) publishes
 * a cache link only after its complete borrowed operand view is prepared.
 */
#ifndef AMA_STATIC_RESULT_OPERANDS_INTERNAL_H
#define AMA_STATIC_RESULT_OPERANDS_INTERNAL_H

#include "../internal.h"

/** One published source/environment preparation with no application state.
 * All pointers borrow result/binding/term owners; the result arena owns the link.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) makes the
 * complete cache entry visible only after successful operand preparation.
 */
struct StaticResultOperands {
    StaticResultOperands *next;
    const StaticResultSource *source;
    const StaticTermApplication *prepared;
};

/** Collect a registered source's local original inputs without following bodies.
 * @param[in,out] plan Own partial records and first errors on any refusal.
 * @param[in] source Registered source validated by the preparation owner.
 * @param[out] output Borrow immutable operands on success; untouched on failure.
 * @return Whether binding/source/term preparation succeeded, without evaluation.
 * [TPL-08](../../../../../spec/language/templates.md#tpl-08) retains original
 * expressions and lexical environments instead of substituting source text.
 */
bool static_result_collect_operands(StaticResultPlan *plan,
    const StaticResultSource *source, const StaticTermApplication **output);

#endif
