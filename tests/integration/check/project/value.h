/**
 * @file
 * @brief Reject any accidental C header compilation during check-only acceptance.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../docs/implementation/library-tests-and-build.md)
 */
#error A check-only command must not compile C
