/**
 * @file
 * @brief Compose native integer construction, literal decoding and read-only conversion helpers.
 *
 * Test allocator/observer macros are optional fixture instrumentation. Normal
 * companion compilation uses malloc/free and no observers. No size constructor
 * invokes a decimal formatter, literal parser or the registered default callback.
 */
#include "integer.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifndef AMA_INTEGER_ALLOCATE
#define AMA_INTEGER_ALLOCATE(size) malloc(size)
#define AMA_INTEGER_RELEASE(pointer) free(pointer)
#endif
#ifndef AMA_INTEGER_OBSERVE_DEFAULT
#define AMA_INTEGER_OBSERVE_DEFAULT() ((void)0)
#define AMA_INTEGER_OBSERVE_LITERAL() ((void)0)
#define AMA_INTEGER_OBSERVE_SIZE() ((void)0)
#endif

#include "../support/integer/storage.c"
#include "../support/integer/literal.c"
#include "../support/integer/conversion.c"
