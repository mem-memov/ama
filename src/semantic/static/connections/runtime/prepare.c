/** @file
 * @brief Publish one immutable source-bound runtime module after exact lexical classification.
 * [TYPE-06](../../../../../spec/language/declarations.md#type-06) fixes names before Type solving.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) owns all failed partial preparation.
 */
#include <stdint.h>
#include "../internal.h"

/** Validate module identity before dereference; one source module inventory is
 * shared across actual specialization environments without sharing instance state.
 * [TYPE-06](../../../../../spec/language/declarations.md#type-06) makes lexical bindings source-determined.
 */
static bool original(const StaticConnectionPlan *plan, const StaticModule *candidate) {
    const StaticInventory *inventory = plan->constants->publications->results->guards->applications->terms->inventory;
    for (const StaticModule *module = inventory->first; module; module = module->next) {
        if (module == candidate) { return true; }
    }
    return false;
}

/** Orchestrate syntax events, explicit inventory and one source-order name pass.
 * Publish only a complete lexical result; no Type solution or runtime execution
 * is implied. Unchanged repeated requests return the exact immutable record.
 * [TYPE-06](../../../../../spec/language/declarations.md#type-06) separates binding and constraint phases.
 */
bool static_connection_runtime(StaticConnectionPlan *plan, const StaticModule *source, const StaticRuntimeModule **output) {
    if (!static_connection_dependencies(plan)) { return false; }
    if (output == NULL || !original(plan, source) || source->definition->syntax.sections.kind == MODULE_STATIC) {
        return static_connection_fail(plan, STATIC_CONNECTION_INVALID, (SourceSpan){0}, "Invalid original runtime module or binding receiver");
    }
    for (const StaticRuntimeModule *module = plan->first_runtime; module; module = module->next) {
        if (module->source == source) { *output = module; return true; }
    }
    if (plan->runtime_count == SIZE_MAX) {
        return static_connection_fail(plan, STATIC_CONNECTION_OUT_OF_MEMORY, (SourceSpan){0}, "Runtime module binding count exceeds representable storage");
    }
    StaticRuntimeModule *module = static_connection_allocate(plan, sizeof(*module));
    if (module == NULL) { return false; }
    module->source = source;
    if (!static_runtime_events(plan, module)) { return false; }
    static_runtime_sort(module);
    if (!static_runtime_fields(plan, module) || !static_runtime_bind(plan, module)) { return false; }
    module->next = plan->first_runtime;
    plan->first_runtime = module;
    plan->runtime_count++;
    *output = module;
    return true;
}
