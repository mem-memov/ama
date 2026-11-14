/** @file
 * @brief Execute exact signed integer add through normal typed module fields.
 * [LIB-01](../../../spec/language/library.md#lib-01) keeps these operations out
 * of parser syntax. Resource failures remain technical; predicates remain data.
 */
#include "ama_n_985c22ee9ec0446c5d547170abd2bc15af5493053b2429cab216b488f6aacd33.interface.h"
#include "ama_n_985c22ee9ec0446c5d547170abd2bc15af5493053b2429cab216b488f6aacd33.begin.h"
/** Borrow prepared inputs/results only for this synchronous native activation. */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state,
    const ama_t_21bd726ed8e85edf24f6b23e137013b3671ac2d77325eeaa1928127f05434f4b *parameters,
    ama_t_7d6259de8dc3df144922ecfeb8ca5040c6e4e5373803580cfd07061921cbe72c *results,
    const AmaNativeServices *services, AmaStatus *status) {
    (void)state; (void)services;
    *status = ama_n_1ad46fade0175c4e31ef9315b33c02e79bb0f66fd54f938c04bd36d8838b2a42_add(results->AMA_OUTPUT(0)->payload,
        parameters->AMA_INPUT(0)->payload, parameters->AMA_INPUT(1)->payload);
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
#include "ama_n_985c22ee9ec0446c5d547170abd2bc15af5493053b2429cab216b488f6aacd33.end.h"
