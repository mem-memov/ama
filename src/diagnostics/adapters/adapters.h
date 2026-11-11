#ifndef AMA_DIAGNOSTIC_ADAPTERS_H
#define AMA_DIAGNOSTIC_ADAPTERS_H

#include "diagnostics/record/diagnostic.h"
#include "source/module/catalog.h"
#include "source/identity/identity.h"

/* Adapter outputs borrow the input owner and one caller-provided related slot.
 * Invalid/no-error inputs leave output unchanged. No allocation is needed. */
bool diagnostic_from_module(const ModuleIssue *issue,
    DiagnosticRelated *related, Diagnostic *output);
bool diagnostic_from_identity_collision(const IdentityName *previous,
    const char *requested_name, SourceSpan requested_span,
    DiagnosticRelated *related, Diagnostic *output);

#endif
