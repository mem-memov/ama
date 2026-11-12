/**
 * @file
 * @brief Match only an exact generated entry or its dot-suffixed compiler family.
 *
 * [STACK-04](../../../../spec/implementation/compiler.md#stack-04)
 * Unrelated helpers sharing a prefix are not entry evidence; the family is tied to the verified GCC configuration.
 */
#include "internal.h"

#include <string.h>

bool ama_listing_matches(AmaByteView symbol, AmaByteView entry) {
    if (symbol.length < entry.length || memcmp(symbol.data, entry.data, entry.length) != 0) {
        return false;
    }
    return symbol.length == entry.length || symbol.data[entry.length] == '.';
}
