#include "ama_n_1f65df889e9651be26f93dd77aaeb32b0b4dff69a779ce533964a874fbdb7dbd.interface.h"
#include "ama_n_1f65df889e9651be26f93dd77aaeb32b0b4dff69a779ce533964a874fbdb7dbd.begin.h"

AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state,
    const ama_t_b0b283f85496c21cc3cef8cef581a4073a3cb36c135fd2c53e8017f471537cc2 *parameters, ama_t_f78ce0059cb8ac8f642db0ba036f79d0e72fb1061b9390301bc33e68e2df21ac *results,
    const AmaNativeServices *services, AmaStatus *status) {
    (void)state; (void)parameters; (void)results; (void)services;
    results->AMA_OUTPUT(0)->payload->value = parameters->AMA_INPUT(0)->payload->value < parameters->AMA_INPUT(1)->payload->value;
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

#include "ama_n_1f65df889e9651be26f93dd77aaeb32b0b4dff69a779ce533964a874fbdb7dbd.end.h"
