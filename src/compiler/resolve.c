/** @file
 * @brief Compose guarded ordinary semantic admission independently of executable storage planning.
 * [COMP-02](../../spec/implementation/compiler.md#comp-02) requires one checked semantic pipeline;
 * [ARCH-03](../../spec/implementation/compiler.md#arch-03) retains failed initialized owners;
 * [TOOL-04](../../spec/implementation/tools-and-conformance.md#tool-04) checks entry-free prepared source domains.
 */
#include "internal.h"
#include "semantic/specializations/check.h"

/** Source closure is already loaded/admitted, and roots are explicit for an
 * entry-free snapshot. Executable callers may select their actual loaded entry.
 * No program plan, filesystem input capture or execution is performed here.
 * [COMP-02](../../spec/implementation/compiler.md#comp-02) preserves common Type/operation/constant checks.
 */
bool compiler_frontend_resolve(CompilerFrontend *frontend, const SemanticNativeAuthoring *authoring) {
    if (!frontend->specialization_initialized) {
        const ProjectModule *roots[] = {frontend->project.entry};
        if (!compiler_frontend_specialize(frontend, roots, 1)) { return false; }
    } else if (!frontend->specializations.valid) {
        frontend->error = frontend->specializations.error;
        return false;
    }
    semantic_program_init(&frontend->semantic, &frontend->project, frontend->allocator);
    frontend->semantic_initialized = true;
    if (semantic_specializations_check(&frontend->semantic, &frontend->specializations.closure, authoring) != SEMANTIC_OK
        || semantic_constants(&frontend->semantic) != SEMANTIC_OK) {
        frontend->error = frontend->semantic.error;
        return false;
    }
    return true;
}
