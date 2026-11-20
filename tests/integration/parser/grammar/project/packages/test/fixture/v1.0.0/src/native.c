/**
 * @file
 * @brief Instantiate Unicode protocol helper fixtures under one explicit native naming context.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../../../../../../../docs/implementation/library-tests-and-build.md)
 */
#include "native.h"
#include "hooks.inc"
