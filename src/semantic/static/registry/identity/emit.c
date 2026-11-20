/** @file
 * @brief Serialize canonical owner maps and postorder nominal DAG records without expanding sharing.
 * [TPL-05](../../../../../spec/language/templates.md#tpl-05) preserves all inputs/captures/residuals.
 * [NAME-01](../../../../../spec/implementation/compiler.md#name-01) preserves root/category distinctions.
 * [NAME-02](../../../../../spec/implementation/compiler.md#name-02) requires deterministic framing.
 */
#include <string.h>
#include "internal.h"

/** Full logical path prefixes determine the specified project/standard root;
 * retaining both the explicit tag and original path prevents punctuation erasure.
 * [NAME-01](../../../../../spec/implementation/compiler.md#name-01).
 */
static void origin(StaticKeyWriter *writer, const StaticModule *module) {
    const char *path = module->definition->source->logical_path;
    static_key_bytes(writer, path[1] == '/' ? "2" : "1", 1);
    static_key_text(writer, path, strlen(path));
}

/** Original formal names are exact borrowed UTF-8 spans, not source ordinals.
 * [TPL-05](../../../../../spec/language/templates.md#tpl-05) retains original parameter identities.
 */
static void formal(StaticKeyWriter *writer, const StaticSymbol *symbol) {
    SourceSpan name = symbol->name;
    static_key_text(writer, (const char *)source_bytes(name.source) + name.start, name.end - name.start);
}

/** Canonical child references retain actual Type/Template categories. A repeated
 * shared subtree is referenced, not flattened into exponentially repeated text.
 * [TPL-05](../../../../../spec/language/templates.md#tpl-05) preserves full nominal values.
 */
static void bindings(const StaticKeyPlan *plan, StaticKeyWriter *writer, const StaticTermCapture *input) {
    for (; input; input = input->next) {
        const StaticKeyNode *node = static_key_find(plan, input->value);
        if (node == NULL || !node->complete) { writer->status = STATIC_KEY_INVALID; return; }
        formal(writer, input->formal);
        static_key_bytes(writer, input->value->category == STATIC_CATEGORY_TYPE ? "T" : "M", 1);
        static_key_number(writer, node->ordinal);
    }
}

/** Current ordinary constructors have Type-valued residual formals, whose full
 * metadata is name, Type category and empty nested signature. Producers are not
 * constructors and never enter this table.
 * [TPL-05](../../../../../spec/language/templates.md#tpl-05) forbids reducing signatures to arity.
 */
static void residuals(StaticKeyWriter *writer, const StaticNominalTerm *nominal) {
    static_key_number(writer, nominal->residual_count);
    for (const StaticTermResidual *item = nominal->first_residual; item; item = item->next) {
        formal(writer, item->formal);
        static_key_bytes(writer, "T", 1);
        static_key_number(writer, 0);
    }
}

/** Version one contains kind/root/path, full original input map, then the
 * reachable postorder table. Every reference names one earlier completed child;
 * root references are checked against the counted table by the format contract.
 * [NAME-02](../../../../../spec/implementation/compiler.md#name-02) fixes field order/version.
 */
StaticKeyStatus static_key_emit(const StaticKeyPlan *plan, StaticKeyWriter *writer) {
    static const char prefix[] = "AMA-STATIC-KEY:1;";
    static_key_bytes(writer, prefix, sizeof(prefix) - 1);
    static_key_bytes(writer, plan->environment->producer_applied ? "P" : "O", 1);
    origin(writer, plan->environment->definition);
    static_key_number(writer, plan->environment->definition->parameter_count);
    bindings(plan, writer, static_key_inputs(plan->environment));
    static_key_number(writer, plan->count);
    for (const StaticKeyNode *node = plan->first; node; node = node->next_order) {
        const StaticTerm *value = node->value;
        static_key_bytes(writer, value->category == STATIC_CATEGORY_TYPE ? "T" : "M", 1);
        origin(writer, value->as.nominal.origin);
        static_key_number(writer, value->as.nominal.capture_count);
        bindings(plan, writer, value->as.nominal.first_capture);
        residuals(writer, &value->as.nominal);
    }
    return writer->status;
}
