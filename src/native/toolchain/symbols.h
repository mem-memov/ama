/**
 * @file
 * @brief Declare allocation-free forbidden-entry-family checks on verified nm output.
 *
 * [STACK-04](../../../spec/implementation/compiler.md#stack-04)
 * Native entry functions and Ama control-label functions must not remain as
 * definitions, references or compiler specializations. Inspection uses unstripped non-LTO objects.
 */
#ifndef AMA_NATIVE_TOOLCHAIN_SYMBOLS_H
#define AMA_NATIVE_TOOLCHAIN_SYMBOLS_H

#include "../abi/bytes.h"

/** Verification result distinguishes a clear symbol family, a surviving entry and malformed listing evidence. Component contract: [STACK-04](../../../spec/implementation/compiler.md#stack-04). */
typedef enum {
    AMA_SYMBOLS_CLEAR,
    AMA_SYMBOLS_ENTRY_REMAINS,
    AMA_SYMBOLS_MALFORMED
} AmaSymbolCheck;

/* Parse GNU nm --format=posix output from one unstripped, non-LTO object.
 * Names are exact registered nonempty C entry identifiers, including control labels
 * that must not become C functions. No storage is retained or allocated.
 */
/** Read the verified finite nm --format=posix listing and exact forbidden entry names.
 * @return Malformed record takes precedence, otherwise any exact/dot-family entry remains, or clear. Views remain caller-owned; no allocation or symbol-name normalization.
 * Component contract: [STACK-04](../../../spec/implementation/compiler.md#stack-04). */
AmaSymbolCheck ama_check_entry_symbols(AmaByteView listing,
    const AmaByteView *entry_names, size_t entry_count);

#endif
