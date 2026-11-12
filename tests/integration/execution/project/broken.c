/**
 * @file
 * @brief Provide a deliberately invalid native body to prove failed builds preserve published executables.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../docs/implementation/library-tests-and-build.md)
 */
#error "Native compilation must fail before publication"
