/**
 * @file
 * @brief Increment one retained index value after checking the maximum.
 *
 * All inputs/views are operation-scoped; generated interfaces establish the native naming context. See the scalar library contract for the explicit standard-root binding.
 * @see [Fixture and coverage contract](../../../../docs/implementation/library-tests-and-build.md)
 * [LIB-01](../../../../spec/language/library.md#lib-01)
 * binds this native scalar by its explicit standard-root nominal identity.
 * [ERROR-01](../../../../spec/implementation/diagnostics.md#error-01)
 * returns range overflow as one owned failure value, while native errors propagate.
 */
#include "ama_n_ea2cd332e0582b42404bf7b9b788ce10b617de1f4fe93839f9246c0f143e1b38.interface.h"
#include "ama_n_ea2cd332e0582b42404bf7b9b788ce10b617de1f4fe93839f9246c0f143e1b38.begin.h"

/**
 * Execute this explicit native body synchronously over borrowed parameter/result views. Set
 * status on every return and retain no view; authored effects are defined by the enclosing
 * module fixture.
 */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state,
    const ama_t_b771769dffd4a2816f626286f095519852ea2f3ed47ae1f21a7e61ca8f3b7536 *parameters, ama_t_e440d0d02bbebe296fb803f88cd057a5cef57932a8469616fdfe06c322805044 *results,
    const AmaNativeServices *services, AmaStatus *status) {
    (void)state; (void)parameters; (void)results; (void)services;
    size_t value = parameters->AMA_INPUT(0)->payload->value;
    if (value == SIZE_MAX) {
        static const unsigned char description[] = "Index successor exceeds the supported range";
        *status = ama_n_edcb3e348eb3d3a06b885e12e77fcb6bd0b86d265d2077a038d757a4f799f515_set(results->AMA_OUTPUT(1),
            (AmaByteView){description, sizeof(description) - 1});
        return;
    }
    results->AMA_OUTPUT(0)->payload->value = value + 1;
    *status = ama_n_edcb3e348eb3d3a06b885e12e77fcb6bd0b86d265d2077a038d757a4f799f515_clear(results->AMA_OUTPUT(1));
}

/**
 * Initialize one fresh native State with its success default; no prior live resource may occupy
 * destination.
 */
AmaStatus AMA_NATIVE(initialize)(AMA_NATIVE(State) *destination) {
    *destination = (AMA_NATIVE(State)){0};
    return ama_status_success();
}
/**
 * Release only successfully constructed native State. Retained scalar modules own no resource;
 * fixture-specific injected failures are returned after cleanup.
 */
AmaStatus AMA_NATIVE(destroy)(AMA_NATIVE(State) *state) {
    (void)state;
    return ama_status_success();
}
/**
 * Copy source native data into fresh destination storage without copying runtime identity or
 * transient execution links; source stays unchanged.
 */
AmaStatus AMA_NATIVE(clone)(AMA_NATIVE(State) *destination, const AMA_NATIVE(State) *source) {
    *destination = *source;
    return ama_status_success();
}

#include "ama_n_ea2cd332e0582b42404bf7b9b788ce10b617de1f4fe93839f9246c0f143e1b38.end.h"
