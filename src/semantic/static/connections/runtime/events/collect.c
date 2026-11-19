/** @file
 * @brief Prepare original runtime operand events from the shared source statement catalog.
 * [TYPE-06](../../../../../../spec/language/declarations.md#type-06) orders actual operands before Type solving.
 * [ARCH-03](../../../../../../spec/implementation/compiler.md#arch-03) owns partial event preparation.
 */
#include <assert.h>
#include <stdint.h>
#include "../../internal.h"

/** Prepare before linking; original syntax/source owners outlive every event.
 * [TYPE-06](../../../../../../spec/language/declarations.md#type-06) retains exact occurrence identity.
 */
static bool append(StaticConnectionPlan *plan, StaticRuntimeModule *module, StaticRuntimeEvent event) {
    if (module->event_count == SIZE_MAX) {
        return static_connection_fail(plan, STATIC_CONNECTION_OUT_OF_MEMORY, event.site, "Runtime source event count exceeds representable storage");
    }
    StaticRuntimeEvent *record = static_connection_allocate(plan, sizeof(*record));
    if (record == NULL) { return false; }
    *record = event;
    if (module->last_event) { module->last_event->next = record; }
    else { module->first_event = record; }
    module->last_event = record;
    module->event_count++;
    return true;
}

/** Each owning node contributes finite local events only. Nested owners already
 * have catalog entries; neither aliases nor Type expressions create runtime uses.
 * [TYPE-06](../../../../../../spec/language/declarations.md#type-06) excludes static/foreign names.
 */
static bool events(StaticConnectionPlan *plan, StaticRuntimeModule *module, const StaticStatement *statement) {
    const AstNode *node = statement->syntax;
    StaticRuntimeEvent event = {.node = node, .context = statement->context, .site = node->span};
    if (node->kind == AST_DECLARATION) {
        event.kind = STATIC_RUNTIME_DECLARATION;
        event.site = node->as.declaration.name->span;
        if (!append(plan, module, event)) { return false; }
        event.kind = STATIC_RUNTIME_INITIALIZER;
        for (const AstInitializer *item = node->as.declaration.first; item; item = item->next) {
            event.source = item->source;
            event.site = item->source->span;
            if (!append(plan, module, event)) { return false; }
        }
        return true;
    }
    if (node->kind == AST_ASSIGNMENT) { event.kind = STATIC_RUNTIME_ASSIGNMENT; }
    else if (node->kind == AST_CALL) {
        event.kind = STATIC_RUNTIME_CALL;
        if (!append(plan, module, event)) { return false; }
        event.kind = STATIC_RUNTIME_BINDING;
        for (const AstArgument *argument = node->as.call.first; argument; argument = argument->next) {
            event.argument = argument;
            event.site = argument->span;
            if (!append(plan, module, event)) { return false; }
        }
        return true;
    } else if (node->kind == AST_BRANCH || node->kind == AST_LOOP) {
        event.kind = STATIC_RUNTIME_CONDITION;
        event.site = node->as.control.condition->span;
    } else if (node->kind == AST_FOREACH) {
        event.kind = STATIC_RUNTIME_FOREACH;
        event.site = node->as.traversal.collection;
    } else { return true; }
    return append(plan, module, event);
}

/** Collection order is raw ownership order; the separate stable sort establishes
 * the lexical order. Every catalog node and local edge is visited once, with no
 * host recursion, lookup, Type constraints or runtime execution.
 * [TYPE-06](../../../../../../spec/language/declarations.md#type-06) separates source order from AST grouping.
 */
bool static_runtime_events(StaticConnectionPlan *plan, StaticRuntimeModule *module) {
    assert(module->first_event == NULL && module->event_count == 0);
    if (!static_connection_dependencies(plan)) { return false; }
    for (const StaticStatement *statement = module->source->first_statement; statement; statement = statement->next) {
        if (!events(plan, module, statement)) { return false; }
    }
    return true;
}
