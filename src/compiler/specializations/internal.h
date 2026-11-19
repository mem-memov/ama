/** @file
 * @brief Separate definition-time source checks, selected roots and whole-Type closure orchestration.
 * [COMP-02](../../../spec/implementation/compiler.md#comp-02) composes semantic stages explicitly.
 */
#ifndef AMA_COMPILER_SPECIALIZATION_INTERNAL_H
#define AMA_COMPILER_SPECIALIZATION_INTERNAL_H
#include "analysis.h"

/** Bind original ordinary runtime names throughout loaded definitions without
 * choosing any Type arguments or registering ordinary instances.
 * [TPL-06](../../../spec/language/templates.md#tpl-06) requires definition-time lexical checking.
 */
bool compiler_specialization_definitions(CompilerSpecialization *);
/** Register exactly the borrowed original ordinary root list. Preserve first
 * error and create no fallback/default root for an invalid selection.
 * [TPL-11](../../../spec/language/templates.md#tpl-11) attributes required expansion.
 */
bool compiler_specialization_roots(CompilerSpecialization *, const ProjectModule *const *, size_t);
#endif
