/**
 * @file
 * @brief Bind maintained integer protocols to counted allocation and isolated native names.
 *
 * The fixed ..integer namespace is checked against the current root-aware
 * identity producer. Generated standard-root bridge binding is checked by the scalar-library suite.
 */
#ifndef AMA_INTEGER_ABI_TEST_COMMON_C
#define AMA_INTEGER_ABI_TEST_COMMON_C
#include "../../../source/common.c"
#include <stdio.h>
#include "ast/ast.h"
#include "parser/abi/abi.h"
#include "source/identity/identity.h"

static size_t integer_default_calls, integer_literal_calls, integer_size_calls;
#define AMA_NATIVE(local_name) ama_n_1ad46fade0175c4e31ef9315b33c02e79bb0f66fd54f938c04bd36d8838b2a42_##local_name
#define AMA_INTEGER_ALLOCATE(size) test_allocate(NULL, size)
#define AMA_INTEGER_RELEASE(pointer) test_release(NULL, pointer)
#define AMA_INTEGER_OBSERVE_DEFAULT() (++integer_default_calls)
#define AMA_INTEGER_OBSERVE_LITERAL() (++integer_literal_calls)
#define AMA_INTEGER_OBSERVE_SIZE() (++integer_size_calls)
#include "project/stdlib/integer.c"

/** Reset native construction observations before each independently counted case. */
static void setup_integer(void) {
    setup_test();
    integer_default_calls = integer_literal_calls = integer_size_calls = 0;
}

/** Construct one size and prove its exact inverse without any default/literal callback. */
static void roundtrip_size(size_t value) {
    AMA_NATIVE(State) state;
    AmaStatus status = AMA_NATIVE(from_size)(&state, value);
    assert(status.code == AMA_OK && state.live && !state.negative);
    size_t result = 0;
    status = AMA_NATIVE(to_index)(&state, &result);
    assert(status.code == AMA_OK && result == value);
    assert(integer_default_calls == 0 && integer_literal_calls == 0);
    status = AMA_NATIVE(destroy)(&state);
    assert(status.code == AMA_OK && !state.live && live_allocations == 0);
}
#endif
