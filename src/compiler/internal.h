/**
 * @file
 * @brief Separate source closure loading from semantic and resolved-plan completion.
 *
 * [ARCH-03](../../spec/implementation/compiler.md#arch-03)
 * Borrow roots/options during one attempt and retain diagnostics in the frontend owner.
 * [TPL-13](../../spec/language/templates.md#tpl-13) requires native definition
 * admission before authoring/header preparation.
 * [TPL-06](../../spec/language/templates.md#tpl-06) keeps open catalog definitions symbolic.
 * [TOOL-04](../../spec/implementation/tools-and-conformance.md#tool-04) separates
 * plan eligibility from filesystem input sealing.
 * [TPL-11](../../spec/language/templates.md#tpl-11) validates exact selected Type roots.
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
/** Collect source-scoped static identities and reject native own formals before
 * capture, ABI proposals or semantic/header preparation. Retain owners for all
 * error records through frontend destruction; call once after a valid Project.
 * Component contract: [TPL-13](../../spec/language/templates.md#tpl-13),
 * [ARCH-03](../../spec/implementation/compiler.md#arch-03).
 */
bool compiler_frontend_admit(CompilerFrontend *);
/** Own guarded Type analysis for exactly the explicit ordinary roots after
 * source/native admission. Roots must include the loaded entry module that
 * finish will plan; standalone definition-only analysis uses the child owner.
 * Check once; false retains source-backed diagnostics
 * and safe teardown. Successful Type closure still needs ordinary admission.
 * [TPL-06](../../spec/language/templates.md#tpl-06) separates inference and checks.
 * [TPL-11](../../spec/language/templates.md#tpl-11) preserves selected roots.
 */
bool compiler_frontend_specialize(CompilerFrontend *, const ProjectModule *const *, size_t);
/** Explicit export/schema requests require every named root; catalog definition
 * checks require the entry plus concrete ordinary definitions. Open templates
 * and static producers remain definitions until actual applications require them.
 * [TPL-06](../../spec/language/templates.md#tpl-06) validates required specializations.
 */
typedef enum { COMPILER_ROOTS_EXPLICIT, COMPILER_ROOTS_CONCRETE_DEFINITIONS } CompilerRootPolicy;
/** Prepare exact original module roots from the caller's loaded name set, always
 * including the selected entry. Array storage belongs to the frontend Project
 * arena; no runtime object is created. Refusal retains owned partial metadata.
 * [TPL-11](../../spec/language/templates.md#tpl-11) forbids guessed/default roots.
 */
bool compiler_frontend_select_roots(CompilerFrontend *, const char *const *, size_t, CompilerRootPolicy);
/** Capture the successfully loaded immutable project and exact entry policy before semantic completion. Component contract: [ARCH-03](../../spec/implementation/compiler.md#arch-03). */
bool compiler_frontend_capture(CompilerFrontend *, const ModuleRoot *, size_t);
/** Resolve admitted guarded roots, ordinary operations and constants without
 * requiring an executable entry/storage plan. Caller selects explicit roots for
 * an entry-free captured catalog. No input graph is captured or published.
 * [COMP-02](../../spec/implementation/compiler.md#comp-02) keeps source and executable consumers on one semantic implementation.
 */
bool compiler_frontend_resolve(CompilerFrontend *, const SemanticNativeAuthoring *);
/** Resolve ordinary semantics and build the exact program plan without capturing
 * or sealing filesystem inputs. plan.valid is the executable eligibility proof;
 * composite frontend.valid remains unchanged under
 * [TOOL-04](../../spec/implementation/tools-and-conformance.md#tool-04).
 */
bool compiler_frontend_plan(CompilerFrontend *, const SemanticNativeAuthoring *);

/**
 * Analyze the loaded closure using an optional explicit authoring contract, then build resolved
 * plans. Record initialization before fallible work and retain the first stage diagnostic.
 * Component contract: [ARCH-03](../../spec/implementation/compiler.md#arch-03). */
bool compiler_frontend_finish(CompilerFrontend *frontend,
    const SemanticNativeAuthoring *authoring);

/** Copy one explicit frontend configuration payload and attach it to the loaded entry before sealing; failure retains an allocation-free diagnostic. Component contract: [ARCH-03](../../spec/implementation/compiler.md#arch-03). */
bool compiler_frontend_input(CompilerFrontend *, const char *, const unsigned char *, size_t);

#endif
