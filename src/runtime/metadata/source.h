/**
 * @file
 * @brief Declare immutable generated source locations using bounded UTF-8 byte views.
 *
 * [DEBUG-01](../../../spec/implementation/diagnostics.md#debug-01)
 * Borrow program-lifetime text; lines and byte columns are one-based and operation selector zero means absent.
 */
#ifndef AMA_RUNTIME_METADATA_SOURCE_H
#define AMA_RUNTIME_METADATA_SOURCE_H

#include "native/abi/bytes.h"

/* Immutable generated UTF-8 map entry; line/byte-column are one-based.
 * Map selector zero means no operation; valid operation IDs start at one.
 */
/** Borrowed program-lifetime UTF-8 source location; line and byte column are one-based, with selector zero reserved outside this record. Component contract: [DEBUG-01](../../../spec/implementation/diagnostics.md#debug-01). */
typedef struct {
    AmaByteView qualified_module;
    AmaByteView physical_path;
    size_t line;
    size_t byte_column;
} AmaSourceLocation;

#endif
