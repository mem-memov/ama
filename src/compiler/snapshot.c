/** @file
 * @brief Validate an explicitly captured source catalog through the shared compiler without probing future native paths.
 * [TOOL-04](../../spec/implementation/tools-and-conformance.md#tool-04) validates staged bytes before publication;
 * [ARCH-03](../../spec/implementation/compiler.md#arch-03) retains all failed owners;
 * [TPL-06](../../spec/language/templates.md#tpl-06) keeps unused open definitions symbolic.
 */
#include <assert.h>
#include "internal.h"

/** The caller's capture/version owner is authoritative. This operation owns
 * parsing/admission/closure/semantics only; its unsealed input graph cannot
 * stand in for physical input capture. Composite executable readiness stays false;
 * no entry, plan or runtime instance is invented.
 * [TOOL-04](../../spec/implementation/tools-and-conformance.md#tool-04) requires
 * projected native paths to remain unexecuted before publication.
 */
bool compiler_frontend_check_snapshot(CompilerFrontend *frontend, const char *const *required, size_t count) {
    assert(!frontend->attempted);
    frontend->attempted = true;
    if (project_load_catalog_set(&frontend->project,required,count) != PROJECT_OK) {
        frontend->error = frontend->project.error;
        return false;
    }
    if (!compiler_frontend_admit(frontend)
        || !compiler_frontend_select_roots(frontend,required,count,COMPILER_ROOTS_CONCRETE_DEFINITIONS)
        || !compiler_frontend_resolve(frontend,NULL)) { return false; }
    return true;
}
