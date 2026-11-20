/**
 * @file
 * @brief Declare a distinct nominal grammar fixture native value with equal local spelling.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../../../../../../../docs/implementation/library-tests-and-build.md)
 */
#ifndef PARSER_OTHER_NATIVE_FIXTURE_H
#define PARSER_OTHER_NATIVE_FIXTURE_H
#include <stdbool.h>
#include "native/abi/bytes.h"
#include "native/status/status.h"
#include "native/toolchain/gcc.h"

/** Second distinct nominal native length payload with the same local type spelling. */
typedef struct { size_t length; } AMA_NATIVE(NativeValue);
#endif
