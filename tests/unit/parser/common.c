/**
 * @file
 * @brief Share the structural layout/section fixture declarations without duplicating ownership.
 *
 * The enclosing leaf explicitly runs assertion cases and owns setup/teardown; shared fixtures retain no state across independent test executables.
 * @see [Fixture and coverage contract](../../../docs/implementation/library-tests-and-build.md)
 */
#ifndef AMA_PARSER_TEST_COMMON
#define AMA_PARSER_TEST_COMMON
#include "../lexer/common.c"
#include "parser/module/sections.h"
#endif
