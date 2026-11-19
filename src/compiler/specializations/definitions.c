/** @file
 * @brief Check lexical runtime bindings even in open or otherwise uninstantiated loaded ordinary definitions.
 * [TPL-06](../../../spec/language/templates.md#tpl-06) requires source/name checking at definition time.
 */
#include "internal.h"

/** Reuse the immutable runtime binder catalog. This operation generates no
 * Type substitutions, incoming roots or runtime instances. Static graph scopes
 * were checked by the preceding source inventory component.
 * [TYPE-06](../../../spec/language/declarations.md#type-06) fixes original names before inference.
 */
bool compiler_specialization_definitions(CompilerSpecialization *analysis) {
    for (const StaticModule *module = analysis->inventory->first; module; module = module->next) {
        if (module->definition->syntax.sections.kind == MODULE_STATIC) { continue; }
        const StaticRuntimeModule *runtime;
        if (!static_connection_runtime(&analysis->connections, module, &runtime)) {
            analysis->error = analysis->connections.error;
            return false;
        }
    }
    return true;
}
