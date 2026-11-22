/** @file
 * @brief Check an independently selected captured entry without granting physical build authority.
 * [TOOL-04](../../spec/implementation/tools-and-conformance.md#tool-04) forbids
 * probing future native/configuration paths;
 * [ARCH-03](../../spec/implementation/compiler.md#arch-03) preserves one-attempt failure owners;
 * [PKG-08](../../spec/project/packages.md#pkg-08) requires actual executable test eligibility.
 */
#include <string.h>
#include "internal.h"
#include "source/path/logical.h"
bool compiler_frontend_check_entry_snapshot(CompilerFrontend *frontend, const char *entry) {
    if (!frontend || frontend->attempted || !frontend->project.catalog.built || !entry
        || !module_path_canonical((const unsigned char *)entry, strlen(entry))) { return false; }
    frontend->attempted = true;
    if (project_load_catalog(&frontend->project, entry, NULL, 0) != PROJECT_OK) {
        frontend->error = frontend->project.error; return false;
    }
    return compiler_frontend_admit(frontend) && compiler_frontend_plan(frontend, NULL);
}
