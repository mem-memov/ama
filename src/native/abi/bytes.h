/**
 * @file
 * @brief Declare bounded borrowed literal bytes independently of string terminators.
 *
 * [CABI-03](../../../spec/implementation/native-interface.md#cabi-03)
 * Embedded zero bytes retain their length; no native callback owns or retains the view.
 */
#ifndef AMA_NATIVE_ABI_BYTES_H
#define AMA_NATIVE_ABI_BYTES_H

#include <limits.h>
#include <stddef.h>

_Static_assert(CHAR_BIT == 8, "Ama native ABI requires eight-bit bytes");

/* Borrow immutable bytes; literal constructors receive UTF-8. NULL requires length 0.
 * Embedded zero bytes and an absent terminator do not shorten this view.
 */
/** Bounded borrowed immutable bytes, including embedded NUL; null data is valid only at zero length. Literal callback views end with that callback. */
typedef struct {
    const unsigned char *data;
    size_t length;
} AmaByteView;

#endif
