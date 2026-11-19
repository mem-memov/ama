/** @file
 * @brief Compose ordinary Type evidence under actual checked incoming provenance.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) refines only actual facts.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) separates incoming paths from metadata and admission.
 * [TPL-06](../../../../../spec/language/templates.md#tpl-06) keeps Type facts separate from ordinary operation admission.
 */
#ifndef AMA_STATIC_CLOSURE_ORDINARY_PASS_H
#define AMA_STATIC_CLOSURE_ORDINARY_PASS_H
#include "../../connections/plan.h"

/** Separate closed field Types from remaining transfer/capability obligations.
 * Neither flag certifies required callee closure or executable admission.
 * [TPL-06](../../../../../spec/language/templates.md#tpl-06) requires ordinary checks after inference.
 */
typedef struct {
    bool changed;
    bool fields_closed;
    bool transfers_ready;
} StaticOrdinaryPass;

/** Submit one finite pass of original annotation/default/assignment/foreach/call
 * evidence, then check actual field Types. Pending work does not skip independent
 * equations. Borrow all inputs; allocate only through existing child owners.
 * On refusal leave output unchanged, preserve first diagnostics and retain partial
 * work for normal child teardown. Does not create a callee context or certify a
 * candidate. Progress counts semantic facts and proofs, never scratch allocation.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) preserves unknown targets.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) preserves real ancestry.
 */
bool static_closure_ordinary_pass(StaticConnectionPlan *, const StaticResultContext *, StaticOrdinaryPass *);
#endif
