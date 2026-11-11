#ifndef AMA_NATIVE_TOOLCHAIN_SYMBOLS_H
#define AMA_NATIVE_TOOLCHAIN_SYMBOLS_H

#include "../abi/bytes.h"

typedef enum {
    AMA_SYMBOLS_CLEAR,
    AMA_SYMBOLS_ENTRY_REMAINS,
    AMA_SYMBOLS_MALFORMED
} AmaSymbolCheck;

/* Parse GNU nm --format=posix output from one unstripped, non-LTO object.
 * Names are nonempty C entry identifiers. No storage is retained or allocated.
 */
AmaSymbolCheck ama_check_native_symbols(AmaByteView listing,
    const AmaByteView *entry_names, size_t entry_count);

#endif
