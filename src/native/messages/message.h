/** @file
 * @brief Encode explicitly declared bounded data records without transporting Ama identity or execution state.
 * [PAR-03](../../../spec/language/async-and-parallelism.md#par-03) requires
 * self-contained typed messages and validation before publication. This native
 * profile supports ordered scalar/list fields of conditions, integers, text and
 * binary bytes; providers explicitly bind its schema to their typed Ama modules.
 */
#ifndef AMA_NATIVE_MESSAGES_MESSAGE_H
#define AMA_NATIVE_MESSAGES_MESSAGE_H
#include <stdlib.h>
#include "native/abi/bytes.h"
#include "native/abi/code.h"

/** Data profiles only: no handle, address, continuation or resource-pointer kind exists. */
typedef enum {
    AMA_MESSAGE_CONDITION = 1, AMA_MESSAGE_INTEGER, AMA_MESSAGE_TEXT, AMA_MESSAGE_BYTES
} AmaNativeMessageKind;

/** Declared field bounds. Scalars require maximum_count=1; lists require a
 * positive maximum. maximum_bytes bounds each byte string or integer magnitude;
 * conditions require zero. A zero text/bytes limit permits only empty data.
 */
typedef struct {
    AmaNativeMessageKind kind;
    bool list;
    size_t maximum_count, maximum_bytes;
} AmaNativeMessageField;

/** Immutable application protocol declaration, borrowed only during each call.
 * Name is a nonempty exact byte identity, version is positive, and the provider
 * must give each incompatible layout a new identity/version. All byte/count
 * limits are checked in addition to the positive complete-envelope limit.
 */
typedef struct {
    AmaByteView name;
    uint32_t version;
    const AmaNativeMessageField *fields;
    size_t field_count, maximum_wire_bytes;
} AmaNativeMessageSchema;

/** Check immutable native schema configuration without allocation or wire data.
 * Channels use this before privately copying the declaration; success grants no
 * ownership or longer borrow. Invalid declarations return technical InvalidState.
 */
AmaStatus ama_native_message_schema_check(const AmaNativeMessageSchema *);

/** Borrowed scalar. Condition flag is 0/1 with empty bytes. Integer flag is
 * sign 0/1 and bytes is canonical nonempty big-endian magnitude (no negative zero).
 * Text/bytes use flag zero. No borrow survives encoding or decoded-owner disposal.
 */
typedef struct { unsigned flag; AmaByteView bytes; } AmaNativeMessageValue;

/** Ordered scalar/list value; scalar count is exactly one and empty lists are valid. */
typedef struct { const AmaNativeMessageValue *items; size_t count; } AmaNativeMessageFieldValue;

/** Ordinary protocol outcomes are separate from technical allocation/configuration failure. */
typedef enum {
    AMA_MESSAGE_ACCEPTED, AMA_MESSAGE_UNSUPPORTED_FORMAT, AMA_MESSAGE_SCHEMA_MISMATCH,
    AMA_MESSAGE_MALFORMED, AMA_MESSAGE_LIMIT
} AmaNativeMessageReason;

/** A non-OK status is technical failure; reason is meaningful only for OK status. */
typedef struct { AmaStatus status; AmaNativeMessageReason reason; } AmaNativeMessageResult;

/** Move-only native envelope. Nonnull data owns malloc/free-compatible storage,
 * never a borrowed view cast into an owner. Code credit outlives every queued
 * byte/release dependency; ownership moves require ending prior data borrows.
 */
typedef struct { unsigned char *data; size_t length; AmaNativeCodeLease code; } AmaNativeMessageBytes;

/** Move-only decoded native data, independent of the input envelope and its schema storage.
 * fields/items are read-only borrows until drop; values/wire own their allocations.
 * These pointers are native data ownership, never serialized addresses or Ama references.
 */
typedef struct {
    AmaNativeMessageFieldValue *fields;
    AmaNativeMessageValue *values;
    unsigned char *wire;
    size_t field_count;
    AmaNativeCodeLease code;
} AmaNativeMessageDecoded;

/** Consume ownership before release; empty/repeated drop is harmless. Loader barriers
 * still cover the final code release's return under [EMBED-06](../../../spec/implementation/c-library.md#embed-06).
 */
static inline void ama_native_message_bytes_drop(AmaNativeMessageBytes *owner) {
    if (owner == NULL) { return; }
    AmaNativeMessageBytes value = *owner;
    *owner = (AmaNativeMessageBytes){0};
    free(value.data);
    ama_native_code_drop(&value.code);
}

/** Release all independent decoded storage before its final creator-image credit. */
static inline void ama_native_message_decoded_drop(AmaNativeMessageDecoded *owner) {
    if (owner == NULL) { return; }
    AmaNativeMessageDecoded value = *owner;
    *owner = (AmaNativeMessageDecoded){0};
    free(value.fields);
    free(value.values);
    free(value.wire);
    ama_native_code_drop(&value.code);
}

/** Validate all input data before allocating an independent envelope. Output must
 * be entirely empty and changes only on acceptance. Provider is explicitly supplied;
 * its zero value denotes process-resident code. Source/schema must not mutate
 * during this synchronous call. Expected data rejection preserves the output.
 */
AmaNativeMessageResult ama_native_message_encode(const AmaNativeMessageSchema *,
    const AmaNativeMessageFieldValue *, const AmaNativeCodeProvider *, AmaNativeMessageBytes *);

/** Validate complete borrowed wire data without allocation, code acquisition or
 * output mutation. Channels use this boundary before accepting an entire message;
 * validation grants no lifetime or Ama access and does not consume the envelope.
 */
AmaNativeMessageResult ama_native_message_validate(const AmaNativeMessageSchema *, AmaByteView);

/** Validate format, exact name/version, every field/limit and complete consumption
 * before allocation. Build independent native data privately; allocation failure
 * cleans its prefix and preserves empty output. Typed Ama providers subsequently
 * construct fresh owned fields before publishing a successful module result.
 */
AmaNativeMessageResult ama_native_message_decode(const AmaNativeMessageSchema *, AmaByteView,
    const AmaNativeCodeProvider *, AmaNativeMessageDecoded *);
#endif
