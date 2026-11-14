/**
 * @file
 * @brief Add two retained index values after an exact overflow precheck.
 *
 * All inputs/views are operation-scoped; generated interfaces establish the native naming context. See the scalar library contract for the explicit standard-root binding.
 * @see [Fixture and coverage contract](../../../../docs/implementation/library-tests-and-build.md)
 * [LIB-01](../../../../spec/language/library.md#lib-01)
 * binds this native scalar by its explicit standard-root nominal identity.
 * [ERROR-01](../../../../spec/implementation/diagnostics.md#error-01)
 * returns range overflow as one owned failure value, while native errors propagate.
 */
#include "ama_n_4b7d139e9d2d9c02adde472229b47a0ec73fe89a23f4be2ec30229a09782ae26.interface.h"
#include "ama_n_4b7d139e9d2d9c02adde472229b47a0ec73fe89a23f4be2ec30229a09782ae26.begin.h"

/**
 * Execute this explicit native body synchronously over borrowed parameter/result views. Set
 * status on every return and retain no view; authored effects are defined by the enclosing
 * module fixture.
 */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state,
    const ama_t_3cc59f9bb1ebff10ae39251f59c87d299294d04f0bc54e0d368e126cc41cb9d4 *parameters, ama_t_b6fc641f3c96b8ed640a205d1651aadff76910315a759e9a384c4223fe7fd163 *results,
    const AmaNativeServices *services, AmaStatus *status) {
    (void)state; (void)parameters; (void)results; (void)services;
    size_t left = parameters->AMA_INPUT(0)->payload->value;
    size_t right = parameters->AMA_INPUT(1)->payload->value;
    if (left > SIZE_MAX - right) {
        static const unsigned char description[] = "Index sum exceeds the supported range";
        *status = ama_n_edcb3e348eb3d3a06b885e12e77fcb6bd0b86d265d2077a038d757a4f799f515_set(results->AMA_OUTPUT(1),
            (AmaByteView){description, sizeof(description) - 1});
        return;
    }
    results->AMA_OUTPUT(0)->payload->value = left + right;
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

#include "ama_n_4b7d139e9d2d9c02adde472229b47a0ec73fe89a23f4be2ec30229a09782ae26.end.h"
