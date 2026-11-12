/**
 * @file
 * @brief Declare allocation-free surviving-entry-family checks on verified nm output.
 *
 * [STACK-04](../../../spec/implementation/compiler.md#stack-04)
 * Entry definitions, references or compiler specializations prevent certification; unstripped non-LTO object workflow is required.
 */
#ifndef AMA_NATIVE_TOOLCHAIN_SYMBOLS_H
#define AMA_NATIVE_TOOLCHAIN_SYMBOLS_H

#include "../abi/bytes.h"

/** Verification result distinguishes a clear symbol family, a surviving entry and malformed listing evidence. */
typedef enum {
    AMA_SYMBOLS_CLEAR,
    AMA_SYMBOLS_ENTRY_REMAINS,
    AMA_SYMBOLS_MALFORMED
} AmaSymbolCheck;

/* Parse GNU nm --format=posix output from one unstripped, non-LTO object.
 * Names are nonempty C entry identifiers. No storage is retained or allocated.
 */
/** Read the verified finite nm --format=posix listing and exact generated entry names.
 * @return Malformed record takes precedence, otherwise any exact/dot-family entry remains, or clear. Views remain caller-owned; no allocation or symbol-name normalization.
 */
AmaSymbolCheck ama_check_native_symbols(AmaByteView listing,
    const AmaByteView *entry_names, size_t entry_count);

#endif
