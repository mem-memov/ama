/**
 * @file
 * @brief Recognize complete static names without splitting them into module/member lookups.
 *
 * [TPL-01](../../../spec/language/templates.md#tpl-01) makes the complete
 * three-dot-prefixed spelling one atomic name with ordinary named components.
 * [COMP-01](../../../spec/implementation/compiler.md#comp-01) forbids consulting
 * declarations to recognize its syntax.
 */
#include "../internal.h"

/** Consume exactly three leading dots and nonempty adjoining named components.
 * Each component strictly advances the cursor; dots are source separators, not
 * member selection or another filesystem root. Invalid suffixes are not repaired.
 * Component contract: [TPL-01](../../../spec/language/templates.md#tpl-01)
 * rejects bare/four-dot prefixes and empty or padded components before lookup.
 */
bool parser_static_name(SourceSpan text) {
    if (!source_span_valid(text) || text.end - text.start < 4) {
        return false;
    }
    const unsigned char *bytes = source_bytes(text.source);
    if (bytes[text.start] != '.' || bytes[text.start + 1] != '.'
        || bytes[text.start + 2] != '.') {
        return false;
    }
    size_t cursor = text.start + 3;
    for (;;) {
        SourceSpan component;
        if (!lex_name(text, &cursor, &component)) {
            return false;
        }
        if (cursor == text.end) {
            return true;
        }
        if (bytes[cursor++] != '.') {
            return false;
        }
    }
}
