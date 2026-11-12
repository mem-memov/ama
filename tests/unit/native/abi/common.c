/**
 * @file
 * @brief Share native ABI types, assertions and protocol fixture declarations.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../docs/implementation/library-tests-and-build.md)
 */
#ifndef AMA_TEST_NATIVE_ABI_COMMON_C
#define AMA_TEST_NATIVE_ABI_COMMON_C

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include "native/abi/bytes.h"
#include "native/toolchain/gcc.h"

#endif
