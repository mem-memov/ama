/**
 * @file
 * @brief Read the nested macro value whose header dependency must trigger recompilation.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../../../../docs/implementation/library-tests-and-build.md)
 */
#include "value.h"

#ifndef EXTRA_VALUE
#define EXTRA_VALUE 0
#endif

/**
 * Return the selected fixture value whose nested header dependency is exercised by the enclosing
 * rebuild test.
 */
int selected_value(void) {
    return FIXTURE_VALUE + EXTRA_VALUE;
}
