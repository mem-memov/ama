#include "internal.h"

#include <string.h>

bool ama_listing_matches(AmaByteView symbol, AmaByteView entry) {
    if (symbol.length < entry.length || memcmp(symbol.data, entry.data, entry.length) != 0) {
        return false;
    }
    return symbol.length == entry.length || symbol.data[entry.length] == '.';
}
