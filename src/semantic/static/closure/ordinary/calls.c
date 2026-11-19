/** @file
 * @brief Enumerate actual source-owned call events without skipping unresolved targets or branches.
 * [TPL-06](../../../../../spec/language/templates.md#tpl-06) checks all source operations.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) keeps unknown targets pending.
 */
#include "internal.h"

/** The lexical event catalog follows owning source edges, including selected
 * calls and refusal bodies, without duplicating control aliases. A Pending call
 * never suppresses another event's equations. Failure preserves child diagnostics.
 * [TYPE-06](../../../../../spec/language/declarations.md#type-06) fixes original operations before solving.
 */
bool static_closure_ordinary_calls(StaticConnectionPlan *plan, const StaticResultContext *actual,
    const StaticTypedScope *scope, StaticOrdinaryPass *report) {
    for (const StaticRuntimeEvent *event = scope->runtime->first_event; event; event = event->next) {
        if (event->kind != STATIC_RUNTIME_CALL) { continue; }
        bool changed;
        StaticResultOutcome outcome;
        if (!static_connection_bindings(plan, actual, event->node, &changed, &outcome)) { return false; }
        report->changed |= changed;
        report->transfers_ready &= outcome == STATIC_RESULT_READY;
    }
    return true;
}
