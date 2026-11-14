/** @file
 * @brief Execute an empty scalar entry and compose independent integer native protocols.
 * [LIB-01](../../spec/language/library.md#lib-01) keeps numerical interpretation
 * in this module; [PROTO-03](../../spec/implementation/native-interface.md#proto-03)
 * constructs list lengths directly in the same ordinary integer representation.
 */
#include "ama_n_1ad46fade0175c4e31ef9315b33c02e79bb0f66fd54f938c04bd36d8838b2a42.interface.h"
#include "ama_n_1ad46fade0175c4e31ef9315b33c02e79bb0f66fd54f938c04bd36d8838b2a42.begin.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#define AMA_INTEGER_ALLOCATE(size) malloc(size)
#define AMA_INTEGER_RELEASE(pointer) free(pointer)
#define AMA_INTEGER_OBSERVE_DEFAULT() ((void)0)
#define AMA_INTEGER_OBSERVE_LITERAL() ((void)0)
#define AMA_INTEGER_OBSERVE_SIZE() ((void)0)
#include "integer+/native/storage.c"
#include "integer+/native/transport.c"
#include "integer+/native/literal.c"
#include "integer+/native/conversion.c"
#include "integer+/native/arithmetic.c"
#undef AMA_INTEGER_ALLOCATE
#undef AMA_INTEGER_RELEASE
#undef AMA_INTEGER_OBSERVE_DEFAULT
#undef AMA_INTEGER_OBSERVE_LITERAL
#undef AMA_INTEGER_OBSERVE_SIZE
/** Scalar entry preserves its existing mathematical value and returns normally. */
AMA_NATIVE_ENTRY void AMA_NATIVE(entry)(AMA_NATIVE(State) *state,
    const ama_t_bcd1d16fda7ed02f7cd466d97da6f7535fee89f2ea8609d605474094056d79cb *parameters,
    ama_t_533695d5f44abe1bb851197c6b130879daf6a40d01ded4990a8d3ea900a3bcca *results,
    const AmaNativeServices *services, AmaStatus *status) {
    (void)state; (void)parameters; (void)results; (void)services;
    *status = ama_status_success();
}
#include "ama_n_1ad46fade0175c4e31ef9315b33c02e79bb0f66fd54f938c04bd36d8838b2a42.end.h"
