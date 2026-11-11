#ifndef AMA_NATIVE_ABI_BYTES_H
#define AMA_NATIVE_ABI_BYTES_H

#include <limits.h>
#include <stddef.h>

_Static_assert(CHAR_BIT == 8, "Ama native ABI requires eight-bit bytes");

/* Borrow immutable UTF-8 bytes for one constructor call. NULL requires length 0.
 * Embedded zero bytes and an absent terminator do not shorten this view.
 */
typedef struct {
    const unsigned char *data;
    size_t length;
} AmaByteView;

#endif
