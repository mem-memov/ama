/**
 * @file
 * @brief Declare the failing native fixture payload independently of its deliberately invalid body.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../../../../../../docs/implementation/library-tests-and-build.md)
 */
/**
 * Module-qualified native payload only; generated interface supplies AMA_NATIVE and required SDK
 * declarations. No persistent Ama identity may be hidden in this State.
 */
struct AMA_NATIVE(State) { unsigned char value; };
