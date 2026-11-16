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
 * Component contract: [ARCH-03](../../spec/implementation/compiler.md#arch-03). */
bool compiler_frontend_load(CompilerFrontend *frontend, const ModuleRoot *roots,
    size_t count, const char *entry);
/** Load one entry and explicit additional modules into a shared registry before
 * configuration tracking and semantic completion; never execute an export.
 * Component contract: [ARCH-03](../../spec/implementation/compiler.md#arch-03). */
bool compiler_frontend_load_required(CompilerFrontend *, const ModuleRoot *, size_t,
    const char *, const char *const *, size_t);
/** Capture the successfully loaded immutable project and exact entry policy before semantic completion. Component contract: [ARCH-03](../../spec/implementation/compiler.md#arch-03). */
bool compiler_frontend_capture(CompilerFrontend *, const ModuleRoot *, size_t);
/**
 * Analyze the loaded closure using an optional explicit authoring contract, then build resolved
 * plans. Record initialization before fallible work and retain the first stage diagnostic.
 * Component contract: [ARCH-03](../../spec/implementation/compiler.md#arch-03). */
bool compiler_frontend_finish(CompilerFrontend *frontend,
    const SemanticNativeAuthoring *authoring);

/** Copy one explicit frontend configuration payload and attach it to the loaded entry before sealing; failure retains an allocation-free diagnostic. Component contract: [ARCH-03](../../spec/implementation/compiler.md#arch-03). */
bool compiler_frontend_input(CompilerFrontend *, const char *, const unsigned char *, size_t);

#endif
