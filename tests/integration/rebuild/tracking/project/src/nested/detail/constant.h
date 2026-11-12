/**
 * @file
 * @brief Define the nested value changed by incremental dependency tests.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../../../../../docs/implementation/library-tests-and-build.md)
 */
#define FIXTURE_VALUE 7
