#include "ama_n_5e311a9cf1f92a83196f5dd28ba2ea70e210b11c7498bee2fb43b1805ee5eaa4.interface.h"
#include "ama_n_5e311a9cf1f92a83196f5dd28ba2ea70e210b11c7498bee2fb43b1805ee5eaa4.begin.h"

AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state,
    const ama_t_0ef1ce19a11994f99c0d593c908e6f4ebce966f14aa012436870961a785ac020 *parameters, ama_t_a37c0aaf4f8a49981a6f71341d4c74191a1ffc758e6bab0349c7dbb933f80c8c *results,
    const AmaNativeServices *services, AmaStatus *status) {
    (void)state;
    (void)parameters;
    results->AMA_OUTPUT(0)->payload->value = services->caller_refusal;
    *status = ama_status_success();
}

AmaStatus AMA_NATIVE(initialize)(AMA_NATIVE(State) *destination) {
    destination->reserved = 0;
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

#include "ama_n_5e311a9cf1f92a83196f5dd28ba2ea70e210b11c7498bee2fb43b1805ee5eaa4.end.h"
