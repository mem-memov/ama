/**
 * @file
 * @brief Instantiate the same local protocol helpers under a different native module context.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../../../../../../../docs/implementation/library-tests-and-build.md)
 */
#include "other.h"
#include "hooks.inc"
