/**
 * @file
 * @brief Separate source closure loading from semantic and resolved-plan completion.
 *
 * [ARCH-03](../../spec/implementation/compiler.md#arch-03)
 * Borrow roots/options during one attempt and retain diagnostics in the frontend owner.
 */
#ifndef AMA_COMPILER_FRONTEND_INTERNAL_H
#define AMA_COMPILER_FRONTEND_INTERNAL_H

#include "frontend.h"

/**
 * Require no prior attempt; load the complete exact project closure and retain its diagnostic on
 * failure. This function neither emits nor runs native code.
 */
bool compiler_frontend_load(CompilerFrontend *frontend, const ModuleRoot *roots,
    size_t count, const char *entry);
/**
 * Analyze the loaded closure using an optional explicit authoring contract, then build resolved
 * plans. Record initialization before fallible work and retain the first stage diagnostic.
 */
bool compiler_frontend_finish(CompilerFrontend *frontend,
    const SemanticNativeAuthoring *authoring);

#endif
