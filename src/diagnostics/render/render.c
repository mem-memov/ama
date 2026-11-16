/**
 * @file
 * @brief Render validated primary/related diagnostics and flush explicit output failures.
 *
 * [COMP-04](../../../spec/implementation/compiler.md#comp-04)
 * [SYN-01](../../../spec/language/source-and-names.md#syn-01) gives the empty-body code warning severity.
 * Keep message, cause, expected construct, help and related context distinct.
 */
#include "internal.h"

/** Omit null optional prose; otherwise write its label and escaped text, propagating the first output failure.
 * Component contract: [COMP-04](../../../spec/implementation/compiler.md#comp-04), [SYN-01](../../../spec/language/source-and-names.md#syn-01). */
static bool write_field(FILE *output, const char *label, const char *text) {
    return text == NULL || (fputs(label, output) != EOF
        && diagnostic_write_text(output, text) && fputc('\n', output) != EOF);
}

/** Render one validated primary diagnostic and distinct optional cause/expected/help sections; no source content is guessed.
 * Component contract: [COMP-04](../../../spec/implementation/compiler.md#comp-04), [SYN-01](../../../spec/language/source-and-names.md#syn-01). */
static bool write_primary(FILE *output, const Diagnostic *diagnostic) {
    return diagnostic_write_location(output, diagnostic->primary)
        && fprintf(output, ": %s[%s]: ", diagnostic->code == DIAGNOSTIC_EMPTY_BODY ? "warning" : "error", diagnostic_code_name(diagnostic->code)) >= 0
        && diagnostic_write_text(output, diagnostic->message)
        && fputc('\n', output) != EOF
        && write_field(output, "    subject: ", diagnostic->subject)
        && diagnostic_write_excerpt(output, diagnostic->primary)
        && write_field(output, "    cause: ", diagnostic->cause)
        && write_field(output, "    expected: ", diagnostic->expected)
        && write_field(output, "    help: ", diagnostic->help);
}

/** Render one validated related declaration/path and its explicit role/name, preserving the borrowed source context.
 * Component contract: [COMP-04](../../../spec/implementation/compiler.md#comp-04), [SYN-01](../../../spec/language/source-and-names.md#syn-01). */
static bool write_related(FILE *output, const DiagnosticRelated *related) {
    return diagnostic_write_location(output, related->location)
        && fputs(": note: ", output) != EOF
        && diagnostic_write_text(output, related->role)
        && fputc('\n', output) != EOF
        && write_field(output, "    subject: ", related->name)
        && diagnostic_write_excerpt(output, related->location);
}

DiagnosticRenderStatus diagnostic_render(FILE *output, const Diagnostic *diagnostic) {
    if (output == NULL || !diagnostic_valid(diagnostic)) {
        return DIAGNOSTIC_RENDER_INVALID;
    }
    if (!write_primary(output, diagnostic)) {
        return DIAGNOSTIC_RENDER_IO_ERROR;
    }
    for (size_t index = 0; index < diagnostic->related_count; index++) {
        if (!write_related(output, &diagnostic->related[index])) {
            return DIAGNOSTIC_RENDER_IO_ERROR;
        }
    }
    return fflush(output) == 0 && !ferror(output)
        ? DIAGNOSTIC_RENDER_OK : DIAGNOSTIC_RENDER_IO_ERROR;
}
