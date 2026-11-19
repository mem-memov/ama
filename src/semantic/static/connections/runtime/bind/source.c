/** @file
 * @brief Bind only actual runtime source names and preserve literal/qualified/self independence.
 * [TYPE-06](../../../../../../spec/language/declarations.md#type-06) excludes literal bytes and qualified members from name encounters.
 * [CONST-06](../../../../../../spec/language/declarations.md#const-06) forbids mutable/runtime shared initialization.
 */
#include "../../internal.h"

/** Source category and storage ownership are already known without Type values.
 * Qualified namespaces retain T181's separate provenance/selection obligations;
 * no member or literal bytes enter this module's variable namespace.
 * [TYPE-06](../../../../../../spec/language/declarations.md#type-06) fixes exact source transfer forms.
 */
bool static_runtime_source(StaticConnectionPlan *plan, StaticRuntimeModule *module,
    const StaticRuntimeEvent *event, const AstSource *source) {
    const StaticRuntimeField *initializing = event->kind == STATIC_RUNTIME_INITIALIZER ?
        static_runtime_declaration(module, event->node) : NULL;
    if (source == NULL) {
        return static_connection_fail(plan, STATIC_CONNECTION_INVALID, event->site, "Runtime source event has no original source");
    }
    if (initializing && initializing->role == STATIC_FIELD_CONSTANT && source->kind != AST_LITERAL_SOURCE &&
        source->kind != AST_CONSTANT_SOURCE && source->kind != AST_VALUE_SOURCE) {
        return static_connection_fail(plan, STATIC_CONNECTION_CONFLICT, source->span, "Shared initialization requires literal or immutable constant sources");
    }
    if (source->kind == AST_LITERAL_SOURCE || source->kind == AST_CONSTANT_SOURCE || source->kind == AST_SELF_REFERENCE_SOURCE) { return true; }
    const StaticRuntimeField *field;
    if (!static_runtime_access(plan, module, event, source->access, false, &field)) { return false; }
    if (field->role == STATIC_FIELD_CONSTANT && source->kind != AST_VALUE_SOURCE) {
        return static_connection_fail(plan, STATIC_CONNECTION_CONFLICT, source->span, "Constants cannot supply mutable references or continuations");
    }
    if (source->kind == AST_ACQUIRE_SOURCE && field->reference) {
        return static_connection_fail(plan, STATIC_CONNECTION_CONFLICT, source->span, "Reference acquisition requires owned source storage");
    }
    return true;
}
