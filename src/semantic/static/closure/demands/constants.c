/** @file
 * @brief Resolve direct qualified transfers through independently fixed namespace Types and actual required contexts.
 * [CONST-03](../../../../../spec/language/declarations.md#const-03) forbids receiver-driven namespace inference.
 * [TPL-11](../../../../../spec/language/templates.md#tpl-11) still checks reused namespace definitions.
 */
#include "internal.h"

/** Every deferred transfer keeps its original namespace and destination Type.
 * Namespace closure never reads or executes a constant value. First proof
 * receipts stay in the existing connection owner for subsequent retries.
 * [CONST-06](../../../../../spec/language/declarations.md#const-06) separates declared Type evidence from construction.
 */
bool static_closure_constants(StaticClosurePlan *plan, bool *output) {
    bool ready = true;
    for (const StaticTypedConstant *item = plan->connections->first_constant; item; item = item->next) {
        const StaticConstantSource *source = item->source;
        const StaticResultContext *context;
        bool known;
        if (!static_closure_require(plan, source->context, source->namespace_type,
            source->occurrence->source->span, &context, &known)) { return false; }
        ready &= known;
        if (!known) { continue; }
        bool changed;
        if (!static_connection_constant(plan->connections, item, context, &changed)) {
            (void)static_closure_dependencies(plan); return false;
        }
    }
    *output = ready;
    return true;
}
