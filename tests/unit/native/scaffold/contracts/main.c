/** @file
 * @brief Compile current-source native error contexts and exact standard integer size bridges without rewriting authored companions.
 */
#include "../../../compiler/common.c"
#include <sys/wait.h>
#include <unistd.h>
#include "native/scaffold/internal.h"
#include "support/fixture.c"
#include "support/process.c"
#include "support/providers.c"
#include "support/bridge.c"
#include "support/failures.c"
/** Run current identity/context, typed bridge, skeleton and allocation ownership scenarios. */
void run(void) {
    it_emits_matching_current_source_error_providers_for_root_distinct_reference_cycles();
    it_binds_only_the_exact_standard_integer_header_bridge_and_keeps_six_abi_records();
    it_rejects_missing_or_incompatible_standard_integer_bridge_declarations();
    it_generates_an_explicit_unwritten_standard_bridge_without_overwriting_existing_files();
    it_unwinds_every_current_source_provider_and_scaffold_allocation();
    it_requires_the_library_status_sdk_header_before_model_or_artifact_emission();
}
/** Return success after every explicit contract case completes. */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
