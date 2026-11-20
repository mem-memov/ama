/**
 * @file
 * @brief Declare the first grammar fixture native value under the caller supplied namespace.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../../../../../../../docs/implementation/library-tests-and-build.md)
 */
#ifndef PARSER_NATIVE_FIXTURE_H
#define PARSER_NATIVE_FIXTURE_H
#include <stdbool.h>
#include "native/abi/bytes.h"
#include "native/status/status.h"
#include "native/toolchain/gcc.h"

/** First nominal native length payload under the explicit generated naming context. */
typedef struct { size_t length; } AMA_NATIVE(NativeValue);
#endif
