/** @file
 * @brief Expose source-level lexical and construction visibility independently of concrete Type instances.
 * [NAME-06](../../../../../../spec/language/declarations.md#name-06) preserves original declarations and foreach intervals.
 * [TYPE-06](../../../../../../spec/language/declarations.md#type-06) fixes inferred introductions before substitution.
 */
#include "../../internal.h"

/** Accepted initializer events retain their exact original source extent.
 * [NAME-06](../../../../../../spec/language/declarations.md#name-06) excludes runtime locals while constructing declared state.
 */
const StaticRuntimeField *static_runtime_initializing_at(const StaticRuntimeModule *module, size_t offset) {
    for (const StaticRuntimeEvent *event = module->first_event; event; event = event->next) {
        if (event->kind != STATIC_RUNTIME_INITIALIZER || !event->source) { continue; }
        SourceSpan span = event->source->span;
        if (offset >= span.start && offset < span.end) { return static_runtime_declaration(module, event->node); }
    }
    return NULL;
}

/** The persistent source inventory is the projection's constructor order;
 * lexical locals never acquire constructor storage or an interface ordinal.
 * [NAME-06](../../../../../../spec/language/declarations.md#name-06) permits only preceding own fields during initialization.
 */
static bool preceding(const StaticRuntimeModule *module, const StaticRuntimeField *field,
    const StaticRuntimeField *initializing) {
    bool before = true, found = false, available = false;
    for (const StaticRuntimeField *item = module->first_field; item; item = item->next) {
        if (item == initializing) { before = false; found = true; }
        if (item == field) { available = before; }
    }
    return found && available;
}

/** Inspect accepted source membership, local shadowing, inferred introduction
 * and constructor prefix without reading any runtime value or concrete Type.
 * [NAME-06](../../../../../../spec/language/declarations.md#name-06) makes these predicates lexical; invalid/out-of-scope points return false.
 */
bool static_runtime_available(const StaticRuntimeModule *module, const StaticRuntimeField *field,
    SourceSpan site, const StaticRuntimeField *initializing) {
    if (site.source != module->source->definition->source->text || !source_span_valid(site)) { return false; }
    const StaticRuntimeField *local = static_runtime_local(module, field->name, site);
    if (field->scope) { return !initializing && local == field; }
    if (local || (field->inferred && site.start < field->name.start)) { return false; }
    if (!initializing) { return true; }
    if (initializing->role == STATIC_FIELD_CONSTANT && field->role != STATIC_FIELD_CONSTANT) { return false; }
    return preceding(module, field, initializing);
}
