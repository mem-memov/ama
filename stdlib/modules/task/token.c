/** @file
 * @brief Copy native event identity without granting callback or Ama ownership rights.
 * [ASYNC-05](../../../spec/language/async-and-parallelism.md#async-05)
 * authenticates all token fields when importing a retained completion.
 */
#include "ama_n_220aafe1ccd482880a8ec2e176d30e0bfb06e68054d3a79d710e972a9f679ae0.interface.h"
#include "ama_n_220aafe1ccd482880a8ec2e176d30e0bfb06e68054d3a79d710e972a9f679ae0.begin.h"
/** Initialize an unissued scalar identity. */
AmaStatus AMA_NATIVE(initialize)(AMA_NATIVE(State) *state) { *state = (AMA_NATIVE(State)){0}; return ama_status_success(); }
/** This scalar owns no callback lease or native allocation. */
AmaStatus AMA_NATIVE(destroy)(AMA_NATIVE(State) *state) { (void)state; return ama_status_success(); }
/** Copy identity only; no resource right is duplicated. */
AmaStatus AMA_NATIVE(clone)(AMA_NATIVE(State) *destination, const AMA_NATIVE(State) *source) { *destination = *source; return ama_status_success(); }
/** Report whether an identity was issued, without querying operation completion. */
AmaStatus AMA_NATIVE(condition)(const AMA_NATIVE(State) *state, bool *output) { *output = state->token.generation != 0; return ama_status_success(); }
/** Explicit scalar entry preserves the immutable identity. */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state, const ama_t_0564aaaf510f5b70d5977947bcaf747b8d70ebcc94b8bc2608ae31324937f3aa *parameters,
    ama_t_6422effcc8263baa4da2078d1118a81dcf8855e75f59d02305c34291a54ef51b *results, const AmaNativeServices *services, AmaStatus *status) {
    (void)state; (void)parameters; (void)results; (void)services; *status = ama_status_success();
}
#include "ama_n_220aafe1ccd482880a8ec2e176d30e0bfb06e68054d3a79d710e972a9f679ae0.end.h"
