/**
 * @file
 * @brief Stream normalized real-prefix components followed by untouched pattern components.
 * [PKG-05](../../../../spec/project/packages.md#pkg-05) prevents wildcard
 * cancellation by never passing suffix components through path reduction.
 */
#include "internal.h"

bool selector_next(SelectorCursor *cursor, ModulePathPart *output) {
    if (cursor->base != NULL && cursor->index < cursor->base->count) {
        *output = cursor->base->parts[cursor->index++];
        return true;
    }
    if (cursor->offset == cursor->suffix.end) { return false; }
    const unsigned char *bytes = source_bytes(cursor->suffix.source);
    size_t start = cursor->offset, end = start;
    while (end < cursor->suffix.end && bytes[end] != '/') { end++; }
    *output = (ModulePathPart){bytes + start, end - start, {cursor->suffix.source, start, end}};
    cursor->offset = end == cursor->suffix.end ? end : end + 1;
    return true;
}
