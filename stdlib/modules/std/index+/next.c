#include "ama_n_d0622af49a8e62864b715c1bf3f56938b0759e7f710ce369e65af871199afa23.interface.h"
#include "ama_n_d0622af49a8e62864b715c1bf3f56938b0759e7f710ce369e65af871199afa23.begin.h"

AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state,
    const ama_t_3b1604e3d9e7019d0d3ef287ea105e2345cc4ba53050e11d0e31bc9a039f296e *parameters, ama_t_7804984acf19cc0398f69bfb6ac45849d434048193081c212ae235a52c3257bf *results,
    const AmaNativeServices *services, AmaStatus *status) {
    (void)state; (void)parameters; (void)results; (void)services;
    size_t value = parameters->AMA_INPUT(0)->payload->value;
    if (value == SIZE_MAX) { *status = ama_status_make(AMA_LIMIT_EXCEEDED, NULL, 0); return; }
    results->AMA_OUTPUT(0)->payload->value = value + 1;
    *status = ama_status_success();
}

AmaStatus AMA_NATIVE(initialize)(AMA_NATIVE(State) *destination) {
    *destination = (AMA_NATIVE(State)){0};
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

#include "ama_n_d0622af49a8e62864b715c1bf3f56938b0759e7f710ce369e65af871199afa23.end.h"
