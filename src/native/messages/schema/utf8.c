/** @file
 * @brief Accept exactly scalar UTF-8 bytes without terminators, locale or overlong encodings.
 * [PAR-03](../../../../spec/language/async-and-parallelism.md#par-03) rejects
 * invalid typed text before allocation; binary fields deliberately bypass this grammar.
 */
#include "../internal.h"

/** Every loop consumes a complete scalar; lead-specific first continuation ranges
 * exclude overlongs, surrogates and values above U+10FFFF before remaining reads.
 * Component contract: [PAR-03](../../../../spec/language/async-and-parallelism.md#par-03). */
bool ama_message_utf8(AmaByteView value) {
    for (size_t position = 0; position < value.length;) {
        unsigned lead = value.data[position++];
        if (lead <= 0x7f) { continue; }
        size_t count;
        unsigned lower = 0x80, upper = 0xbf;
        if (lead >= 0xc2 && lead <= 0xdf) { count = 1; }
        else if (lead >= 0xe0 && lead <= 0xef) {
            count = 2;
            if (lead == 0xe0) { lower = 0xa0; }
            if (lead == 0xed) { upper = 0x9f; }
        } else if (lead >= 0xf0 && lead <= 0xf4) {
            count = 3;
            if (lead == 0xf0) { lower = 0x90; }
            if (lead == 0xf4) { upper = 0x8f; }
        } else { return false; }
        if (count > value.length - position || value.data[position] < lower || value.data[position] > upper) { return false; }
        position++;
        for (size_t continuation = 1; continuation < count; continuation++) {
            unsigned byte = value.data[position++];
            if (byte < 0x80 || byte > 0xbf) { return false; }
        }
    }
    return true;
}
