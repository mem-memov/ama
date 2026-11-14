/** @file
 * @brief Validate precisely the Unicode scalar UTF-8 byte grammar before allocation.
 * [CABI-03](../../../../spec/implementation/native-interface.md#cabi-03)
 * supplies counted bytes; validation never relies on a terminator or locale.
 */
#include "internal.h"

/** Decode by explicit lead/first-continuation ranges. Every iteration consumes
 * one complete scalar; remaining-length checks precede every continuation read.
 */
static bool AMA_NATIVE(valid_utf8)(AmaByteView value) {
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
