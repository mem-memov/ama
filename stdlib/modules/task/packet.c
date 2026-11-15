/** @file
 * @brief Import native completion bytes through generated typed owned views.
 * [ASYNC-05](../../../spec/language/async-and-parallelism.md#async-05)
 * keeps callback publication separate from executor-owned Ama updates.
 */
#include "ama_n_e34937e219f84be1f1e0c364ad6e2333fd58e60295c27110ea97a69a88ac4904.interface.h"
#include "ama_n_e34937e219f84be1f1e0c364ad6e2333fd58e60295c27110ea97a69a88ac4904.begin.h"
#include <inttypes.h>
#include <stdio.h>
/** Prepare every allocation before replacing declared children; technical failure publishes no presence. */
AmaStatus AMA_NATIVE(publish)(ama_t_5cd130a48cf871c6976b8d28b7d3db09d549929b4b6ff9649339e81ffadf642e *destination, const AmaNativeEventCompletion *source) {
    char decimal[32];
    int length = snprintf(decimal, sizeof(decimal), "%" PRId64, source->code);
    if (length < 0 || (size_t)length >= sizeof(decimal)) { return ama_status_make(AMA_NATIVE_FAILURE, NULL, 0); }
    ama_n_1ad46fade0175c4e31ef9315b33c02e79bb0f66fd54f938c04bd36d8838b2a42_State code;
    AmaStatus status = ama_n_1ad46fade0175c4e31ef9315b33c02e79bb0f66fd54f938c04bd36d8838b2a42_from_literal(&code, (AmaByteView){(const unsigned char *)decimal, (size_t)length});
    if (status.code != AMA_OK) { return status; }
    ama_n_98bd03817466d12d563a6e9709025da3fa29b415e0e82ad0e6fa0e11f2e7414b_State data;
    status = ama_n_98bd03817466d12d563a6e9709025da3fa29b415e0e82ad0e6fa0e11f2e7414b_from_literal(&data, (AmaByteView){source->buffer.data, source->buffer.length});
    if (status.code != AMA_OK) { (void)ama_n_1ad46fade0175c4e31ef9315b33c02e79bb0f66fd54f938c04bd36d8838b2a42_destroy(&code); return status; }
    status = ama_n_1ad46fade0175c4e31ef9315b33c02e79bb0f66fd54f938c04bd36d8838b2a42_destroy(destination->AMA_OUTPUT(1)->payload);
    if (status.code != AMA_OK) { (void)ama_n_1ad46fade0175c4e31ef9315b33c02e79bb0f66fd54f938c04bd36d8838b2a42_destroy(&code); (void)ama_n_98bd03817466d12d563a6e9709025da3fa29b415e0e82ad0e6fa0e11f2e7414b_destroy(&data); return status; }
    *destination->AMA_OUTPUT(1)->payload = code;
    status = ama_n_98bd03817466d12d563a6e9709025da3fa29b415e0e82ad0e6fa0e11f2e7414b_destroy(destination->AMA_OUTPUT(2)->payload);
    if (status.code != AMA_OK) { (void)ama_n_98bd03817466d12d563a6e9709025da3fa29b415e0e82ad0e6fa0e11f2e7414b_destroy(&data); return status; }
    *destination->AMA_OUTPUT(2)->payload = data;
    destination->AMA_OUTPUT(0)->payload->token = source->token;
    destination->payload->present = true;
    return ama_status_success();
}
/** Presence alone is native state; declared children have generated lifecycle. */
AmaStatus AMA_NATIVE(initialize)(AMA_NATIVE(State) *state) { *state = (AMA_NATIVE(State)){0}; return ama_status_success(); }
/** Generated cleanup retires every owned declared child. */
AmaStatus AMA_NATIVE(destroy)(AMA_NATIVE(State) *state) { (void)state; return ama_status_success(); }
/** Clone scalar presence; generated cloning copies declared owned children independently. */
AmaStatus AMA_NATIVE(clone)(AMA_NATIVE(State) *destination, const AMA_NATIVE(State) *source) { *destination = *source; return ama_status_success(); }
/** Absence is independent of signed operation code and empty data. */
AmaStatus AMA_NATIVE(condition)(const AMA_NATIVE(State) *state, bool *output) { *output = state->present; return ama_status_success(); }
/** An explicit packet entry resets generated children and its presence. */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state, const ama_t_839edde46c8ed8138ac34a6e070c693caa599c7c44a862abb00cd58f033800be *parameters,
    ama_t_07018bfbf0419838f1145ead8a360e4ebf8878972769b1c9e573eb35b0d72572 *results, const AmaNativeServices *services, AmaStatus *status) {
    (void)parameters; (void)results; (void)services; state->present = false; *status = ama_status_success();
}
#include "ama_n_e34937e219f84be1f1e0c364ad6e2333fd58e60295c27110ea97a69a88ac4904.end.h"
