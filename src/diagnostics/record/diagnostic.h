/**
 * @file
 * @brief Expose allocation-free borrowed compiler diagnostics and related source locations.
 *
 * [COMP-04](../../../spec/implementation/compiler.md#comp-04)
 * [SYN-01](../../../spec/language/source-and-names.md#syn-01) adds a nonfatal empty-body category.
 * Keep exact primary/related context separate from human rendering and guessed corrections.
 */
#ifndef AMA_DIAGNOSTIC_RECORD_H
#define AMA_DIAGNOSTIC_RECORD_H

#include "source/text/source.h"

/** Stable compiler diagnostic categories, distinct from runtime native status codes. Component contract: [COMP-04](../../../spec/implementation/compiler.md#comp-04), [SYN-01](../../../spec/language/source-and-names.md#syn-01). */
typedef enum {
    DIAGNOSTIC_INVALID_UTF8,
    DIAGNOSTIC_INVALID_PATH,
    DIAGNOSTIC_MISSING_MODULE,
    DIAGNOSTIC_AMBIGUOUS_MODULE,
    DIAGNOSTIC_FILE_ALIAS,
    DIAGNOSTIC_ORPHAN_DIRECTORY,
    DIAGNOSTIC_UNSUPPORTED_PATH,
    DIAGNOSTIC_IO_FAILURE,
    DIAGNOSTIC_OUT_OF_MEMORY,
    DIAGNOSTIC_IDENTITY_COLLISION,
    DIAGNOSTIC_SYNTAX,
    DIAGNOSTIC_TYPE_MISMATCH,
    DIAGNOSTIC_UNSUPPORTED_FEATURE,
    DIAGNOSTIC_EMPTY_BODY,
    DIAGNOSTIC_CHANGED_INPUT,
    DIAGNOSTIC_RENAME_CONFLICT,
    DIAGNOSTIC_CODE_COUNT
} DiagnosticCode;

/** Borrowed original span, physical path or absent location; explicit path must agree with a loaded source. Component contract: [COMP-04](../../../spec/implementation/compiler.md#comp-04), [SYN-01](../../../spec/language/source-and-names.md#syn-01). */
typedef struct {
    SourceSpan span;
    /* Used for filesystem failures without loaded source. NULL with a span
     * means the source's physical path; NULL without a span means no location. */
    const char *path;
} DiagnosticLocation;

/** Caller-owned related location with explicit role and optional original name. Component contract: [COMP-04](../../../spec/implementation/compiler.md#comp-04), [SYN-01](../../../spec/language/source-and-names.md#syn-01). */
typedef struct {
    DiagnosticLocation location;
    const char *role;
    const char *name;
} DiagnosticRelated;

/** Caller-owned immutable attribution chain supplementing existing related notes.
 * The node and all borrowed source/prose views outlive diagnostic consumption.
 * [COMP-04](../../../spec/implementation/compiler.md#comp-04) retains each actual requiring site without allocating during failure reporting.
 */
typedef struct DiagnosticContext {
    DiagnosticRelated related;
    const struct DiagnosticContext *next;
} DiagnosticContext;

/** Borrowed allocation-free diagnostic view; all strings, sources and related arrays outlive consumers. Component contract: [COMP-04](../../../spec/implementation/compiler.md#comp-04), [SYN-01](../../../spec/language/source-and-names.md#syn-01). */
typedef struct {
    DiagnosticCode code;
    const char *message;
    const char *subject;
    DiagnosticLocation primary;
    const char *cause;
    const char *expected;
    const char *help;
    const DiagnosticRelated *related;
    size_t related_count;
    const DiagnosticContext *contexts;
} Diagnostic;

/** Add a caller-owned attribution node without replacing the primary error or
 * existing notes. A previously attached node is idempotent only for equal
 * location/prose; its contents remain immutable. Reject invalid records/locations
 * and attempted node rebinding without mutation. New nodes must
 * not overlap diagnostic storage, belong to another live diagnostic chain, or
 * expire before any copied diagnostic view. Shared suffixes remain immutable.
 * Nodes render newest attribution first after the original related array.
 * [COMP-04](../../../spec/implementation/compiler.md#comp-04) preserves failure provenance under allocation refusal.
 */
bool diagnostic_add_context(Diagnostic *, DiagnosticContext *, DiagnosticRelated);

/* Allocation-free borrowed record. Text/related arrays/sources must outlive all
 * consumers. Optional prose is NULL, not invented from guessed corrections.
 * Codes and byte spans are the machine interface; rendering is separate. */
/** Validate a borrowed finite diagnostic record without allocation or interpretation of its prose. All pointed storage must remain live during validation/rendering.
 * Component contract: [COMP-04](../../../spec/implementation/compiler.md#comp-04), [SYN-01](../../../spec/language/source-and-names.md#syn-01). */
bool diagnostic_valid(const Diagnostic *diagnostic);
/** Return immutable machine-readable code text, or the explicit invalid-diagnostic sentinel for an out-of-domain code.
 * Component contract: [COMP-04](../../../spec/implementation/compiler.md#comp-04), [SYN-01](../../../spec/language/source-and-names.md#syn-01). */
const char *diagnostic_code_name(DiagnosticCode code);

#endif
