/** @file
 * @brief Execute exact signed integer next through normal typed module fields.
 * [LIB-01](../../../spec/language/library.md#lib-01) keeps these operations out
 * of parser syntax. Resource failures remain technical; predicates remain data.
 */
#include "ama_n_8dd939eb084808ab3ac61992395c0ecf7e8c29e9a5c8b075d507f189c90caa7c.interface.h"
#include "ama_n_8dd939eb084808ab3ac61992395c0ecf7e8c29e9a5c8b075d507f189c90caa7c.begin.h"
/** Borrow prepared inputs/results only for this synchronous native activation. */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state,
    const ama_t_62ea65ad999450fff93c39047d73404a55a41417ae2399b671418daca5dc32f9 *parameters,
    ama_t_0f45132ca7bf92f1f4d83458cd8ab04b11f054624922fdccdc49e1d24de1092a *results,
    const AmaNativeServices *services, AmaStatus *status) {
    (void)state; (void)services;
    ama_n_1ad46fade0175c4e31ef9315b33c02e79bb0f66fd54f938c04bd36d8838b2a42_State one;
    *status = ama_n_1ad46fade0175c4e31ef9315b33c02e79bb0f66fd54f938c04bd36d8838b2a42_from_size(&one, 1);
    if (status->code != AMA_OK) { return; }
    *status = ama_n_1ad46fade0175c4e31ef9315b33c02e79bb0f66fd54f938c04bd36d8838b2a42_add(results->AMA_OUTPUT(0)->payload,
        parameters->AMA_INPUT(0)->payload, &one);
    AmaStatus cleanup = ama_n_1ad46fade0175c4e31ef9315b33c02e79bb0f66fd54f938c04bd36d8838b2a42_destroy(&one);
    if (status->code == AMA_OK) { *status = cleanup; }
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
#include "ama_n_8dd939eb084808ab3ac61992395c0ecf7e8c29e9a5c8b075d507f189c90caa7c.end.h"
