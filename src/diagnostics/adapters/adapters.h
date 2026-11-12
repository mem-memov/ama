/**
 * @file
 * @brief Declare allocation-free source and identity diagnostic adapters.
 *
 * [COMP-04](../../../spec/implementation/compiler.md#comp-04)
 * Preserve source ownership and original context while publishing only validated records.
 */
#ifndef AMA_DIAGNOSTIC_ADAPTERS_H
#define AMA_DIAGNOSTIC_ADAPTERS_H

#include "diagnostics/record/diagnostic.h"
#include "source/module/catalog.h"
#include "source/identity/identity.h"

/* Adapter outputs borrow the input owner and one caller-provided related slot.
 * Invalid/no-error inputs leave output unchanged. No allocation is needed. */
/** Map a real discovery failure to borrowed static prose and at most one caller-owned related slot.
 * @param[out] output Changes only after local record validation; invalid/no-error input publishes nothing.
 * @param[out] related Required only when the issue has a related path; its storage must outlive output consumers.
 * @param[in] issue Borrowed real module discovery failure and optional related path; its catalog outlives output.
 */
bool diagnostic_from_module(const ModuleIssue *issue,
    DiagnosticRelated *related, Diagnostic *output);
/** Publish both original names/declarations for an unequal-key digest collision without allocation.
 * @param[out] output Changes only after complete record validation.
 * @param[out] related Caller-owned slot required through every output consumer; source/name owners also remain live.
 * @param[in] previous Borrowed previously registered unequal full identity and original source location.
 * @param[in] requested_name Borrowed complete original requested name, alive through diagnostic consumption.
 * @param[in] requested_span Original source span of the conflicting request, with a live source owner.
 */
bool diagnostic_from_identity_collision(const IdentityName *previous,
    const char *requested_name, SourceSpan requested_span,
    DiagnosticRelated *related, Diagnostic *output);

#endif
