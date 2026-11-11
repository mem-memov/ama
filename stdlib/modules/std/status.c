#include "ama_n_d8ee4a39419166db9c9c7c32b719f46ef4cb0ca4eefa92e05791136eec4acb67.interface.h"
#include "ama_n_d8ee4a39419166db9c9c7c32b719f46ef4cb0ca4eefa92e05791136eec4acb67.begin.h"

AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state,
    const ama_t_27bacea3c519f66c74472afd298c1ae3baf1c771de122843d3e65a145284c62f *parameters, ama_t_3fa05fc7c79cd3c802bfdce9becee5f41f8aab25b6824ad49c0e986910d0d4e8 *results,
    const AmaNativeServices *services, AmaStatus *status) {
    (void)state;
    (void)parameters;
    (void)results;
    (void)services;
    *status = ama_status_success();
}

AmaStatus AMA_NATIVE(initialize)(AMA_NATIVE(State) *destination) {
    destination->value = ama_status_success();
    return ama_status_success();
}

AmaStatus AMA_NATIVE(destroy)(AMA_NATIVE(State) *state) {
    (void)state;
    return ama_status_success();
}

AmaStatus AMA_NATIVE(clone)(AMA_NATIVE(State) *destination, const AMA_NATIVE(State) *source) {
    *destination = *source;
    return ama_status_success();
}

#include "ama_n_d8ee4a39419166db9c9c7c32b719f46ef4cb0ca4eefa92e05791136eec4acb67.end.h"
