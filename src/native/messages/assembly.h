/** @file
 * @brief Compose the native data codec once in an owning provider translation unit.
 * [PAR-03](../../../spec/language/async-and-parallelism.md#par-03) keeps
 * typed message construction separate from Ama execution and channel delivery.
 */
#ifndef AMA_NATIVE_MESSAGES_ASSEMBLY_H
#define AMA_NATIVE_MESSAGES_ASSEMBLY_H
#include "schema/check.c"
#include "schema/scalar.c"
#include "schema/utf8.c"
#include "wire/cursor.c"
#include "wire/emit.c"
#include "wire/scan.c"
#include "encode/encode.c"
#include "decode/decode.c"
#include "decode/validate.c"
#endif
