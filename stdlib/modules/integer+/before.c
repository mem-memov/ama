/** @file
 * @brief Execute exact signed integer before through normal typed module fields.
 * [LIB-01](../../../spec/language/library.md#lib-01) keeps these operations out
 * of parser syntax. Resource failures remain technical; predicates remain data.
 */
#include "ama_n_838d42b6c4eba5a59ed7dbe1ec978a29cc6b2e8d21c8db307589754cd230644e.interface.h"
#include "ama_n_838d42b6c4eba5a59ed7dbe1ec978a29cc6b2e8d21c8db307589754cd230644e.begin.h"
/** Borrow prepared inputs/results only for this synchronous native activation. */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state,
    const ama_t_1ffb8dee82cb008b0afdb8e2ff378a0337ff20e69385a405c6c2fd02dd92e80c *parameters,
    ama_t_5eed2be0c025b7b887b832a1ac8a6bfa7d897315f7de6e84ebf3741eff20b651 *results,
    const AmaNativeServices *services, AmaStatus *status) {
    (void)state; (void)services;
    int ordering = 0;
    *status = ama_n_1ad46fade0175c4e31ef9315b33c02e79bb0f66fd54f938c04bd36d8838b2a42_compare(parameters->AMA_INPUT(0)->payload,
        parameters->AMA_INPUT(1)->payload, &ordering);
    if (status->code == AMA_OK) { results->AMA_OUTPUT(0)->payload->value = ordering < 0; }
}
/** Own no native resource; values belong to declared inputs/results. */
AmaStatus AMA_NATIVE(initialize)(AMA_NATIVE(State) *destination) {
    *destination = (AMA_NATIVE(State)){0};
    return ama_status_success();
}
/** Generated cleanup finalizes every declared value. */
AmaStatus AMA_NATIVE(destroy)(AMA_NATIVE(State) *state) {
    (void)state;
    return ama_status_success();
}
/** Clone only the resource-free payload; generated cloning owns the field graph. */
AmaStatus AMA_NATIVE(clone)(AMA_NATIVE(State) *destination, const AMA_NATIVE(State) *source) {
    *destination = *source;
    return ama_status_success();
}
#include "ama_n_838d42b6c4eba5a59ed7dbe1ec978a29cc6b2e8d21c8db307589754cd230644e.end.h"
