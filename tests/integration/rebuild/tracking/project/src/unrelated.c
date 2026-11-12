/**
 * @file
 * @brief Provide an unrelated production object that must not rebuild for a disjoint header change.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../../../docs/implementation/library-tests-and-build.md)
 */
int unrelated_value(void) {
    return 0;
}
