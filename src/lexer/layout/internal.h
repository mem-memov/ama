/**
 * @file
 * @brief Declare physical classification and structured layout failure helpers.
 *
 * [ARCH-03](../../../spec/implementation/compiler.md#arch-03)
 * Keep byte-view classification independent of name lookup or AST allocation.
 */
#ifndef AMA_LAYOUT_INTERNAL_H
#define AMA_LAYOUT_INTERNAL_H

#include "layout.h"

/** Classify one valid physical line; mutate only its view bounds/kind/indentation. literal_payloads distinguishes Ama source from ABI records.
 * Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03). */
void layout_classify(LayoutLine *line, bool literal_payloads);
/** Return the retained syntactic input-literal bar offset, or span.end if absent. Operate on source bytes only; label declaration lookup is forbidden.
 * Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03). */
size_t layout_input_literal(SourceSpan text);
/** Record a borrowed exact site/message/expected form without allocation and return the explicit failure status.
 * Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03). */
LayoutStatus layout_failure(SourceLayout *layout, LayoutStatus status,
    SourceSpan span, const char *message, const char *expected);

/** Validate global encoding/TAB/line-ending rules on layout.source before allocating or classifying any physical line. Component contract: [ARCH-03](../../../spec/implementation/compiler.md#arch-03). */
LayoutStatus layout_validate_source(SourceLayout *);

#endif
