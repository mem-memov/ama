/** @file
 * @brief Share bounded schema, scalar and cursor contracts among message codec phases.
 * [PAR-03](../../../spec/language/async-and-parallelism.md#par-03) separates
 * full protocol validation from private allocation and publication.
 */
#ifndef AMA_NATIVE_MESSAGES_INTERNAL_H
#define AMA_NATIVE_MESSAGES_INTERNAL_H
#include <string.h>
#include "message.h"

/** Borrowed cursor with an invariant position<=length; failed reads preserve position. */
typedef struct { AmaByteView bytes; size_t position; } AmaMessageReader;
/** Bounded write/count cursor; a null data pointer performs the exact sizing pass. */
typedef struct { unsigned char *data; size_t position, limit; } AmaMessageWriter;

/** Construct a protocol result without allocating diagnostic message storage. */
static inline AmaNativeMessageResult ama_message_result(AmaNativeMessageReason reason) {
    return (AmaNativeMessageResult){ama_status_success(), reason};
}
/** Keep technical failure provenance independent of ordinary protocol reason. */
static inline AmaNativeMessageResult ama_message_failure(AmaStatus status) {
    return (AmaNativeMessageResult){status, AMA_MESSAGE_ACCEPTED};
}
/** Check one complete scalar's canonical data grammar and declared byte bound. */
AmaNativeMessageReason ama_message_scalar(const AmaNativeMessageField *, AmaNativeMessageValue);
/** Validate Unicode scalar UTF-8 including embedded NUL, independently of locale. */
bool ama_message_utf8(AmaByteView);
/** Read exactly count bytes or leave the cursor unchanged. Output borrows the input. */
bool ama_message_read(AmaMessageReader *, size_t, AmaByteView *);
/** Read one eight-byte network-order unsigned count, rejecting values above SIZE_MAX. */
bool ama_message_read_size(AmaMessageReader *, size_t *);
/** Write/count exactly one slice without overflow or exceeding the complete bound. */
bool ama_message_write(AmaMessageWriter *, AmaByteView);
/** Write/count one fixed network-order unsigned count. */
bool ama_message_write_size(AmaMessageWriter *, size_t);
/** Validate the envelope and count decoded scalar slots; no allocation/output publication. */
AmaNativeMessageReason ama_message_scan(const AmaNativeMessageSchema *, AmaByteView, size_t *, AmaNativeMessageDecoded *);
/** Emit an already validated borrowed record; the same function sizes and writes it. */
bool ama_message_emit(const AmaNativeMessageSchema *, const AmaNativeMessageFieldValue *, AmaMessageWriter *);
#endif
