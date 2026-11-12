/**
 * @file
 * @brief Declare shared typed payload, input and output views for independent toolchain probes.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../../docs/implementation/library-tests-and-build.md)
 */
#include "native/abi/services.h"
#include "native/toolchain/gcc.h"

/** Native probe accumulation state owned by its enclosing instance. */
typedef struct { size_t total; } Payload;
/** Operation-scoped const scalar input view; the entry cannot retain its pointer. */
typedef struct { const size_t *value; } Inputs;
/** Operation-scoped writable scalar result view; the entry cannot retain its pointer. */
typedef struct { size_t *value; } Outputs;
