#ifndef AMA_DIAGNOSTIC_RECORD_H
#define AMA_DIAGNOSTIC_RECORD_H

#include "source/text/source.h"

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
    DIAGNOSTIC_CODE_COUNT
} DiagnosticCode;

typedef struct {
    SourceSpan span;
    /* Used for filesystem failures without loaded source. NULL with a span
     * means the source's physical path; NULL without a span means no location. */
    const char *path;
} DiagnosticLocation;

typedef struct {
    DiagnosticLocation location;
    const char *role;
    const char *name;
} DiagnosticRelated;

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
} Diagnostic;

/* Allocation-free borrowed record. Text/related arrays/sources must outlive all
 * consumers. Optional prose is NULL, not invented from guessed corrections.
 * Codes and byte spans are the machine interface; rendering is separate. */
bool diagnostic_valid(const Diagnostic *diagnostic);
const char *diagnostic_code_name(DiagnosticCode code);

#endif
