/**
 * @file
 * @brief Scan immutable strict INI physical lines without interpreting sections.
 * [PKG-03](../../../../spec/project/packages.md#pkg-03) fixes encoding,
 * physical endings and unpadded column-zero records independently of Ama layout.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) keeps failure sites in original configuration bytes.
 */
#ifndef AMA_SOURCE_CONFIGURATION_LINES_H
#define AMA_SOURCE_CONFIGURATION_LINES_H
#include "source/text/source.h"

/** Original failure site and static explanations; the source owner outlives all views.
 * [COMP-04](../../../../spec/implementation/compiler.md#comp-04) requires an
 * actionable original location, never a location in normalized replacement text.
 */
typedef struct {
    SourceSpan span;
    const char *message;
    const char *expected;
} ConfigurationIssue;

/** Allocation-free cursor borrowing one complete immutable source through end/failure.
 * A failure closes the cursor; the output of next remains unchanged on refusal.
 * [PKG-03](../../../../spec/project/packages.md#pkg-03) rejects malformed bytes
 * globally, including in otherwise ignored comment lines.
 */
typedef struct {
    const SourceText *source;
    size_t offset;
    ConfigurationIssue issue;
    bool failed;
} ConfigurationLines;

/** End and malformed input are distinct; an empty physical line is still a line.
 * [PKG-03](../../../../spec/project/packages.md#pkg-03) distinguishes empty physical lines from malformed padding.
 */
typedef enum {
    CONFIGURATION_LINE,
    CONFIGURATION_END,
    CONFIGURATION_INVALID_LINE
} ConfigurationLineStatus;

/** Initialize a fresh cursor and validate the complete source encoding once.
 * No allocation, byte mutation or filesystem access occurs. NULL source refuses
 * with an empty diagnostic span; otherwise failures identify original bytes.
 * [PKG-03](../../../../spec/project/packages.md#pkg-03) forbids initial BOM/TAB
 * and accepts only LF/CRLF endings. Time O(bytes), auxiliary storage O(1).
 */
bool configuration_lines_begin(ConfigurationLines *lines, const SourceText *source);

/** Publish the next newline-free original span, including empty lines.
 * The cursor must have been initialized by begin; output is valid writable storage.
 * A final nonempty unterminated line is returned exactly once. Nonempty lines
 * cannot start/end with whitespace; nothing is trimmed. Failed/end calls leave
 * output unchanged. Over a complete scan time is O(bytes), storage O(1).
 * [PKG-03](../../../../spec/project/packages.md#pkg-03) requires column zero
 * and forbids trailing whitespace even on full-line INI comments.
 */
ConfigurationLineStatus configuration_line_next(ConfigurationLines *lines, SourceSpan *output);
#endif
