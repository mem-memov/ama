/** @file
 * @brief Select explicit required Type roots from caller requests without instantiating open catalog definitions.
 * [TPL-06](../../spec/language/templates.md#tpl-06) separates definition checks and concrete operations.
 * [TPL-11](../../spec/language/templates.md#tpl-11) requires original selected-root provenance.
 * [SYN-04](../../spec/language/source-and-names.md#syn-04) keeps drafts outside ordinary Type roots.
 * [ARCH-03](../../spec/implementation/compiler.md#arch-03) owns prepared arrays through partial frontend failure.
 */
#include <assert.h>
#include <stdint.h>
#include "internal.h"

/** Empty original Type-formal sets identify concrete source definitions exactly;
 * this predicate never tries arguments, capabilities or a default Type. Static
 * producers are compile-time graphs and cannot be ordinary runtime roots.
 * [TPL-06](../../spec/language/templates.md#tpl-06) permits open definitions without invented instances.
 */
static bool concrete(const CompilerFrontend *frontend, const ProjectModule *module) {
    const StaticModule *definition = static_module_find(&frontend->static_inventory, module);
    assert(definition);
    ParsedModuleKind kind = module->syntax.sections.kind;
    return (kind == MODULE_AMA || kind == MODULE_NATIVE || kind == MODULE_CONSTANTS)
        && definition->parameter_count == 0;
}

bool compiler_frontend_select_roots(CompilerFrontend *frontend, const char *const *names,
    size_t count, CompilerRootPolicy policy) {
    assert(frontend->static_initialized && frontend->static_inventory.valid && !frontend->specialization_initialized);
    assert(policy == COMPILER_ROOTS_EXPLICIT || policy == COMPILER_ROOTS_CONCRETE_DEFINITIONS);
    if (count == SIZE_MAX || count + 1 > SIZE_MAX / sizeof(const ProjectModule *)) {
        frontend->error = (Diagnostic){.code = DIAGNOSTIC_OUT_OF_MEMORY,
            .message = "Required Type-root array exceeds supported storage"};
        return false;
    }
    const ProjectModule **selected = source_arena_allocate(&frontend->project.arena, (count + 1) * sizeof(*selected));
    if (!selected) {
        frontend->error = (Diagnostic){.code = DIAGNOSTIC_OUT_OF_MEMORY,
            .message = "Required Type roots could not allocate storage"};
        return false;
    }
    size_t size = 1;
    selected[0] = frontend->project.entry;
    for (size_t index = 0; index < count; index++) {
        const ProjectModule *module = project_find(&frontend->project, names[index]);
        assert(module);
        if (module == frontend->project.entry) { continue; }
        if (policy == COMPILER_ROOTS_CONCRETE_DEFINITIONS && !concrete(frontend, module)) { continue; }
        selected[size++] = module;
    }
    return compiler_frontend_specialize(frontend, selected, size);
}
