#include "symbols.h"

#include <stdbool.h>
#include "listing/internal.h"

AmaSymbolCheck ama_check_native_symbols(AmaByteView listing,
    const AmaByteView *entry_names, size_t entry_count) {
    size_t position = 0;
    bool remains = false;
    while (position < listing.length) {
        size_t start = position;
        while (position < listing.length && listing.data[position] != '\n') {
            position++;
        }
        AmaByteView line = {listing.data + start, position - start};
        AmaByteView symbol;
        if (!ama_listing_record(line, &symbol)) {
            return AMA_SYMBOLS_MALFORMED;
        }
        for (size_t index = 0; index < entry_count; index++) {
            remains |= ama_listing_matches(symbol, entry_names[index]);
        }
        if (position < listing.length) {
            position++;
        }
    }
    return remains ? AMA_SYMBOLS_ENTRY_REMAINS : AMA_SYMBOLS_CLEAR;
}
