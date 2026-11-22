/** @file
 * @brief Complete ordinary semantics and planning independently of filesystem input sealing.
 * [COMP-02](../../spec/implementation/compiler.md#comp-02) uses one normal
 * Type/operation/storage pipeline; [TOOL-04](../../spec/implementation/tools-and-conformance.md#tool-04)
 * permits captured entry eligibility without future-path I/O;
 * [ARCH-03](../../spec/implementation/compiler.md#arch-03) retains failed plan owners.
 */
#include "internal.h"
bool compiler_frontend_plan(CompilerFrontend *frontend, const SemanticNativeAuthoring *authoring) {
    if (!compiler_frontend_resolve(frontend, authoring)) { return false; }
    program_plan_init(&frontend->plan, &frontend->semantic, frontend->allocator);
    frontend->plan_initialized = true;
    if (program_plan_build(&frontend->plan) != PLAN_OK) {
        frontend->error = frontend->plan.error; return false;
    }
    return true;
}
