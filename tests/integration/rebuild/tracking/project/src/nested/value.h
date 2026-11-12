/**
 * @file
 * @brief Declare the selected fixture value and include its nested dependency.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../../../../../docs/implementation/library-tests-and-build.md)
 */
#ifndef FIXTURE_VALUE_H
#define FIXTURE_VALUE_H

#include "detail/constant.h"

/**
 * Return the selected fixture value whose nested header dependency is exercised by the enclosing
 * rebuild test.
 */
int selected_value(void);

#endif
