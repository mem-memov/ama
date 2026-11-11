#ifndef AMA_RUNTIME_METADATA_SOURCE_H
#define AMA_RUNTIME_METADATA_SOURCE_H

#include "native/abi/bytes.h"

/* Immutable generated UTF-8 map entry; line/byte-column are one-based.
 * Map selector zero means no operation; valid operation IDs start at one.
 */
typedef struct {
    AmaByteView qualified_module;
    AmaByteView physical_path;
    size_t line;
    size_t byte_column;
} AmaSourceLocation;

#endif
