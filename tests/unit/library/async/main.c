/** @file
 * @brief Accept real callback lifetimes across component context disposal and physical shared-library reload.
 */
#include "../common.c"
#include "codegen/codegen.h"
#include "library/artifact/assembly.h"
#include "library/artifact/package.h"
#include "library/artifact/process.h"
#include <unistd.h>
#include "support/files.c"
#include "support/backend.c"
#include "support/component.c"
#include "support/consumer.c"

/** Build all native dependencies explicitly, then run an SDK-free consumer against the actual packaged component. */
static void it_accepts_native_completions_cancellation_restart_and_unload_barriers(void) {
    setup_test();
    directory("generated");
    directory("generated/project");
    directory("generated/component");
    char backend[4096], include[4096];
    build_backend(backend, include);
    install_project();
    LibraryBuild build;
    build_component(&build, backend, include);
    build_consumer(&build, backend, include);
    library_build_destroy(&build);
    destroy_test();
}
/** Run the real dynamic integration; no mock loader or hand-written component ABI substitutes for it. */
void run(void) { it_accepts_native_completions_cancellation_restart_and_unload_barriers(); }
/** Return after all workers, values, contexts and creator images retire in the supported order. */
int main(void) { run(); return EXIT_SUCCESS; }
