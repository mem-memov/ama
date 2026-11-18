/** @file
 * @brief Register original constant namespace reads without selecting an unknown owner's members.
 * [CONST-03](../../../../../spec/language/declarations.md#const-03) forbids receiver/member-driven namespace inference.
 * [TPL-12](../../../../../spec/language/static-results.md#tpl-12) retains static producer syntax separately.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns partial requests.
 */
#include <stdint.h>
#include "../internal.h"
#include "../../bindings/normalize.h"

/** Only an original namespace occurrence owns this operation. Checking identity
 * first prevents copied call payloads from impersonating actual source syntax.
 * [CONST-03](../../../../../spec/language/declarations.md#const-03) preserves original reads.
 */
static bool original(const StaticResultContext *context, const AstNode *candidate) {
    for (const StaticOccurrence *site = context->creation->owner->environment->definition->first_occurrence;
        site; site = site->next) {
        if (site->statement == candidate && site->root && site->receiver == NULL &&
            site->context == STATIC_CONTEXT_NAMESPACE) {
            return candidate->kind == AST_CALL && candidate->as.call.mode == AST_CONSTANT_READ &&
                candidate->as.call.namespace_expression == site->expression;
        }
    }
    return false;
}

/** Raw normalization validates ordinary saved-read ordering/mode but retains
 * null actual slots. No expected signature, member name or receiver chooses an
 * origin. Type obligations remain available for later typed connections.
 * [CONST-03](../../../../../spec/language/declarations.md#const-03) defers member lookup.
 */
bool static_constant_read(StaticConstantPlan *plan, const StaticResultContext *context,
    const AstNode *syntax, const StaticConstantRead **output) {
    if (!static_constant_dependencies(plan)) { return false; }
    StaticResultPlan *results = plan->publications->results;
    if (output == NULL || !static_result_has_context(results, context) || !original(context, syntax)) {
        return static_constant_fail(plan, STATIC_CONSTANT_INVALID, (SourceSpan){0}, "Invalid original constant namespace read or context");
    }
    for (const StaticConstantRead *read = plan->first_read; read; read = read->next) {
        if (read->context == context && read->syntax == syntax) { *output = read; return true; }
    }
    if (plan->read_count == SIZE_MAX) {
        return static_constant_fail(plan, STATIC_CONSTANT_OUT_OF_MEMORY, syntax->span,
            "Constant namespace read count exceeds representable storage");
    }
    StaticResultUse *source;
    BindingApplication *requests;
    if (!static_result_use(results, context, syntax->as.call.namespace_expression, NULL, &source) ||
        !binding_normalize_runtime(results->guards->applications->bindings, NULL, syntax, &requests)) {
        (void)static_constant_dependencies(plan); return false;
    }
    StaticConstantRead *read = static_constant_allocate(plan, sizeof(*read));
    if (read == NULL) { return false; }
    *read = (StaticConstantRead){.context = context, .syntax = syntax, .source = source,
        .requests = requests, .next = plan->first_read};
    plan->first_read = read;
    plan->read_count++;
    *output = read;
    return true;
}
