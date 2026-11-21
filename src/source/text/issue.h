/**
 * @file
 * @brief Share original source-validation failures without coupling path and INI stages.
 * [COMP-04](../../../spec/implementation/compiler.md#comp-04) requires original
 * source locations and actionable expectations; [ARCH-03](../../../spec/implementation/compiler.md#arch-03)
 * keeps these views borrowed instead of allocating new text while reporting OOM.
 */
#ifndef AMA_SOURCE_TEXT_ISSUE_H
#define AMA_SOURCE_TEXT_ISSUE_H
#include "source.h"

/** Borrowed original span and static explanations, without ownership or I/O effects.
 * A missing caller-provided source has an empty span; no file location is invented.
 * Source owners outlive issue consumption. [COMP-04](../../../spec/implementation/compiler.md#comp-04)
 * preserves original bytes independently of normalized replacement spellings.
 */
typedef struct {
    SourceSpan span;
    const char *message;
    const char *expected;
} SourceIssue;
#endif
