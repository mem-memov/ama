/**
 * @file
 * @brief Consume a nested-header-dependent value to make rebuild effects observable.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../../../docs/implementation/library-tests-and-build.md)
 */
int selected_value(void);

/**
 * Return the nested selected fixture value so the parent can observe whether header/source
 * changes reached the linked executable.
 */
int main(void) {
    return selected_value();
}
