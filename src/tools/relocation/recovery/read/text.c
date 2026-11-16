/** @file
 * @brief Decode exact length-framed text and binary recovery data without delimiter searches through payload bytes.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * retains source/recovery bytes exactly and rejects truncated or ambiguous records.
 */
#include <string.h>
#include "../internal.h"

/** A failed reader prefix owns its error source through recovery destruction. */
bool relocation_journal_invalid(RelocationJournalReader *reader, const char *message) {
    RelocationTransaction *transaction = &reader->recovery->transaction;
    relocation_transaction_fail(transaction,RELOCATION_INVALID_JOURNAL,message,NULL);
    if (transaction->error.primary.span.source == NULL) {
        size_t length = source_length(reader->source);
        transaction->error.primary.span = (SourceSpan){reader->source,reader->offset,
            reader->offset < length ? reader->offset+1 : reader->offset};
    }
    return false;
}
/** Length is checked against actual remaining bytes before allocating or moving the cursor. */
static bool length(RelocationJournalReader *reader, size_t *size) {
    uintmax_t value;
    if (!relocation_journal_read_unsigned(reader,&value)) { return false; }
    if (value > SIZE_MAX || value > source_length(reader->source)-reader->offset) {
        reader->truncated = value <= SIZE_MAX;
        return relocation_journal_invalid(reader,"A journal payload length exceeds its remaining bytes");
    }
    *size = (size_t)value;
    return true;
}
/** Text records must remain C strings: embedded NUL cannot silently truncate an authorized filesystem path. */
bool relocation_journal_read_text(RelocationJournalReader *reader, SourceArena *arena, const char **output) {
    size_t size;
    if (!length(reader,&size)) { return false; }
    const unsigned char *bytes = source_bytes(reader->source)+reader->offset;
    if (size == SIZE_MAX || memchr(bytes,0,size) != NULL) {
        return relocation_journal_invalid(reader,"A journal text field contains NUL or has an unrepresentable terminator");
    }
    char *owned = source_arena_allocate(arena,size+1);
    if (owned == NULL) {
        return relocation_transaction_fail(&reader->recovery->transaction,RELOCATION_OUT_OF_MEMORY,"Journal text ownership allocation failed",NULL);
    }
    memcpy(owned,bytes,size);
    owned[size] = 0;
    reader->offset += size;
    *output = owned;
    return true;
}
/** Presence is explicit even when a present field's byte length is zero. */
bool relocation_journal_read_nullable(RelocationJournalReader *reader, SourceArena *arena, const char **output) {
    bool present;
    if (!relocation_journal_read_boolean(reader,&present)) { return false; }
    if (!present) { *output = NULL; return true; }
    return relocation_journal_read_text(reader,arena,output);
}
/** SourceManager copies exact opaque bytes and the diagnostic path, independent of the journal reader's source. */
bool relocation_journal_read_blob(RelocationJournalReader *reader, const char *path, const SourceText **output) {
    size_t size;
    if (!length(reader,&size)) { return false; }
    SourceStatus status = source_manager_add(&reader->recovery->sources,path,
        source_bytes(reader->source)+reader->offset,size,output);
    if (status != SOURCE_OK) {
        return relocation_transaction_fail(&reader->recovery->transaction,RELOCATION_OUT_OF_MEMORY,"Journal snapshot ownership allocation failed",path);
    }
    reader->offset += size;
    return true;
}
