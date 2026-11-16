/** @file
 * @brief Accept one complete recovery inventory and an exact commit/finalization tail.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * distinguishes committed publication from interrupted acknowledgement without interpreting source text again.
 */
#include <string.h>
#include "internal.h"
#include "../transaction/journal/format.h"

/** Consume a fixed token only when every required byte exists and matches. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_journal_read_token(RelocationJournalReader *reader, const char *text) {
    size_t length = strlen(text);
    if (length > source_length(reader->source)-reader->offset
        || memcmp(source_bytes(reader->source)+reader->offset,text,length) != 0) {
        size_t remaining = source_length(reader->source)-reader->offset;
        reader->truncated = remaining < length && memcmp(source_bytes(reader->source)+reader->offset,text,remaining) == 0;
        return relocation_journal_invalid(reader,"The journal format marker is incomplete or unsupported");
    }
    reader->offset += length;
    return true;
}
/** A proper prefix is an interrupted append, not an alternative token or a guessed commit. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
static bool tail(RelocationJournalReader *reader) {
    RelocationRecovery *recovery = reader->recovery;
    RelocationTransaction *transaction = &recovery->transaction;
    size_t remaining = source_length(reader->source)-reader->offset;
    const unsigned char *bytes = source_bytes(reader->source)+reader->offset;
    if (remaining < 7 && memcmp(bytes,"COMMIT\n",remaining) == 0) { return true; }
    if (remaining >= 7 && memcmp(bytes,"COMMIT\n",7) == 0) {
        transaction->committed = true;
        transaction->state = RELOCATION_TRANSACTION_COMMITTED;
        reader->offset += 7;
        remaining -= 7;
        bytes += 7;
        const char *done = "DONE-COMMIT\n";
        size_t length = strlen(done);
        if (remaining <= length && memcmp(bytes,done,remaining) == 0) {
            recovery->finished = remaining == length;
            return true;
        }
    } else {
        const char *done = "DONE-ROLLBACK\n";
        size_t length = strlen(done);
        if (remaining <= length && memcmp(bytes,done,remaining) == 0) {
            recovery->finished = remaining == length;
            recovery->rolled_back = recovery->finished;
            return true;
        }
    }
    return relocation_journal_invalid(reader,"The journal decision tail is malformed");
}
/** The immutable prepared prefix must be complete; only append-only decision/finalization tokens may be interrupted. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_journal_decode(RelocationRecovery *recovery, const SourceText *source) {
    RelocationJournalReader reader = {.recovery=recovery,.source=source};
    recovery->record = source;
    if (!relocation_journal_read_token(&reader,RELOCATION_JOURNAL_MAGIC) || !relocation_journal_read_report(&reader)
        || !relocation_journal_read_actions(&reader) || !relocation_journal_read_token(&reader,"END\n")) { return false; }
    recovery->transaction.journal_length = (off_t)reader.offset;
    if (recovery->transaction.journal_length < 0 || (uintmax_t)recovery->transaction.journal_length != reader.offset) {
        return relocation_journal_invalid(&reader,"The journal prepared offset exceeds the supported file range");
    }
    return tail(&reader);
}
