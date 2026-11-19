/** @file
 * @brief Separate finite source-call enumeration from ordinary-pass orchestration.
 * [COMP-02](../../../../../spec/implementation/compiler.md#comp-02) composes explicit semantic child operations.
 */
#ifndef AMA_STATIC_CLOSURE_ORDINARY_INTERNAL_H
#define AMA_STATIC_CLOSURE_ORDINARY_INTERNAL_H
#include "pass.h"
#include "../../connections/internal.h"

/** Submit each original call's interface equations, retaining Pending targets.
 * Update the private report only; callers publish it after all children succeed.
 * [TPL-10](../../../../../spec/language/templates.md#tpl-10) forbids origin guesses.
 */
bool static_closure_ordinary_calls(StaticConnectionPlan *, const StaticResultContext *,
    const StaticTypedScope *, StaticOrdinaryPass *);
#endif
