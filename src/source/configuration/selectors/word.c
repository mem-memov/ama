/**
 * @file
 * @brief Decide exact embedded-star matching with scalar-prefix Boolean reachability.
 * [PKG-05](../../../../spec/project/packages.md#pkg-05) makes star consume zero
 * or more characters within one component. Each row consumes a pattern scalar;
 * each column consumes one UTF-8 scalar, never an individual continuation byte.
 */
#include <string.h>
#include "internal.h"
#include "source/unicode/unicode.h"

bool selector_word_match(DependencySelectorPart pattern, SelectorWord word, bool *rows, size_t capacity) {
    bool *previous = rows, *current = rows + capacity;
    memset(previous, 0, capacity * sizeof(bool));
    previous[0] = true;
    size_t column = 0;
    for (size_t index = 0; index < pattern.count; index++) {
        uint32_t token = pattern.scalars[index];
        current[0] = token == '*' && previous[0];
        column = 0;
        for (size_t offset = 0; offset < word.length;) {
            uint32_t scalar;
            size_t width;
            if (!utf8_decode(word.bytes + offset, word.length - offset, &scalar, &width)) { return false; }
            column++;
            current[column] = token == '*' ? previous[column] || current[column - 1]
                : previous[column - 1] && scalar == token;
            offset += width;
        }
        bool *swap = previous;
        previous = current;
        current = swap;
    }
    return previous[column];
}
