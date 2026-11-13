/** @file
 * @brief Exhaust bounded ownership/acquisition transitions independently of module dispatch and inject every allocation in a composed trace.
 */
#include "../objects/common.c"
#include "runtime/program/program.h"
#include "runtime/program/calls/call.h"
#include "../program-construction/support/schema.c"
#include "support/state.c"
#include "support/invariants.c"
#include "support/actions/construction.c"
#include "support/actions/replacement.c"
#include "support/actions/acquisition.c"
#include "support/actions/cleanup.c"
#include "support/transitions.c"
#include "support/schedules.c"
/** Execute exhaustive bounded traces and every required allocation failure of the composed scenario. */
void run(void) {
    it_matches_every_bounded_ownership_acquisition_clone_and_replacement_trace();
    it_preserves_the_model_at_every_composed_trace_allocation_failure();
}
/** Return success only after all registered invariant checks pass. */
int main(void) {
    run();
    return EXIT_SUCCESS;
}
