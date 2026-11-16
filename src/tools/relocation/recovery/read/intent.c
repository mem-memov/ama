/** @file
 * @brief Decode durable preparation reservations, ignoring only a provably interrupted final append.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * never reconstructs temporary ownership by scanning plausible directory names.
 */
#include <string.h>
#include "../internal.h"
#include "../../transaction/journal/format.h"
#include "../../internal.h"

/** A reservation becomes authority only after its complete end marker; all earlier copied strings remain arena-owned on failure. */
static bool reserve(RelocationJournalReader *reader) {
    RelocationTransaction *transaction = &reader->recovery->transaction;
    RelocationTransactionSlot value = {.created=true};
    if (!relocation_journal_read_token(reader,"SLOT\n")
        || !relocation_journal_read_text(reader,&transaction->arena,&value.directory)
        || !relocation_journal_read_text(reader,&transaction->arena,&value.parent)
        || !relocation_journal_read_text(reader,&transaction->arena,&value.prepared)
        || !relocation_journal_read_text(reader,&transaction->arena,&value.backup)
        || !relocation_journal_read_token(reader,"END-SLOT\n")) { return false; }
    if (!relocation_recovery_slot_shape(&value,false)
        || relocation_contains(transaction->report.source,value.directory)
        || relocation_contains(transaction->report.destination,value.directory)) {
        return relocation_journal_invalid(reader,"A preparation reservation has invalid private path geometry");
    }
    for (const RelocationTransactionSlot *slot = transaction->slots; slot != NULL; slot = slot->next) {
        if (!slot->acknowledged || relocation_contains(slot->directory,value.directory)
            || relocation_contains(value.directory,slot->directory)) {
            return relocation_journal_invalid(reader,"Preparation reservations overlap or have an unacknowledged predecessor");
        }
    }
    RelocationTransactionSlot *slot = source_arena_allocate(&transaction->arena,sizeof(*slot));
    if (slot == NULL) { return relocation_transaction_fail(transaction,RELOCATION_OUT_OF_MEMORY,"Preparation reservation ownership allocation failed",NULL); }
    value.next = transaction->slots;
    *slot = value;
    transaction->slots = slot;
    return true;
}
/** Identity acknowledgement precedes all copy effects and may occur once for the most recent reservation. */
static bool own(RelocationJournalReader *reader) {
    RelocationTransaction *transaction = &reader->recovery->transaction;
    const char *path;
    struct stat metadata;
    if (!relocation_journal_read_token(reader,"OWN\n")
        || !relocation_journal_read_text(reader,&transaction->arena,&path)
        || !relocation_journal_read_metadata(reader,&metadata)
        || !relocation_journal_read_token(reader,"END-OWN\n")) { return false; }
    RelocationTransactionSlot *slot = transaction->slots;
    if (slot == NULL || slot == transaction->recovery_slot || slot->acknowledged || strcmp(slot->directory,path) != 0) {
        return relocation_journal_invalid(reader,"A preparation acknowledgement has no matching latest reservation");
    }
    RelocationTransactionSlot value = *slot;
    value.metadata = metadata;
    if (!relocation_recovery_slot_shape(&value,true)) { return relocation_journal_invalid(reader,"A preparation acknowledgement lacks private identity metadata"); }
    slot->metadata = metadata;
    slot->acknowledged = true;
    return true;
}
/** Match an operation prefix without consuming bytes so incomplete final tokens remain distinguishable from unknown operations. */
static bool prefix(RelocationJournalReader *reader, const char *token) {
    size_t remaining = source_length(reader->source)-reader->offset, length = strlen(token);
    return memcmp(source_bytes(reader->source)+reader->offset,token,remaining < length ? remaining : length) == 0;
}
/** An incomplete append authorized no new effects; discard only its diagnostic, preserving the last complete offset. */
static bool interrupted(RelocationJournalReader *reader) {
    RelocationTransaction *transaction = &reader->recovery->transaction;
    if (!reader->truncated || transaction->status != RELOCATION_INVALID_JOURNAL) { return false; }
    transaction->status = RELOCATION_OK;
    transaction->error = (Diagnostic){0};
    return true;
}
/** A complete READY authorizes the prepared plan; ABORTED and EOF authorize preparation cleanup only. */
static bool records(RelocationJournalReader *reader, bool *prepared) {
    RelocationRecovery *recovery = reader->recovery;
    RelocationTransaction *transaction = &recovery->transaction;
    while (reader->offset < source_length(reader->source)) {
        bool complete;
        if (prefix(reader,"SLOT\n")) { complete = reserve(reader); }
        else if (prefix(reader,"OWN\n")) { complete = own(reader); }
        else if (prefix(reader,"READY\n")) {
            complete = relocation_journal_read_token(reader,"READY\n");
            if (complete) {
                if (!transaction->slots->acknowledged) { return relocation_journal_invalid(reader,"Prepared publication has an unacknowledged private slot"); }
                *prepared = true;
            }
        } else if (prefix(reader,"ABORTED\n")) {
            complete = relocation_journal_read_token(reader,"ABORTED\n");
            if (complete) { recovery->finished = true; recovery->rolled_back = true; }
        } else { return relocation_journal_invalid(reader,"The preparation intent contains an unknown record"); }
        if (!complete) { return interrupted(reader); }
        transaction->intent_length = (off_t)reader->offset;
        if (transaction->intent_length < 0 || (uintmax_t)transaction->intent_length != reader->offset) {
            return relocation_journal_invalid(reader,"Preparation intent offset exceeds its supported file range");
        }
        if (*prepared || recovery->finished) {
            return reader->offset == source_length(reader->source)
                || relocation_journal_invalid(reader,"Preparation intent has bytes after its terminal marker");
        }
    }
    return true;
}
/** The initial complete scope/root identity is mandatory; it is the only authority for reporting and registered-copy cleanup. */
bool relocation_intent_decode(RelocationRecovery *recovery, const SourceText *source, bool *prepared) {
    RelocationJournalReader reader = {.recovery=recovery,.source=source};
    RelocationTransaction *transaction = &recovery->transaction;
    recovery->record = source;
    recovery->preparing = true;
    *prepared = false;
    if (!relocation_journal_read_token(&reader,RELOCATION_INTENT_MAGIC) || !relocation_journal_read_report(&reader)) { return false; }
    transaction->recovery_slot = relocation_journal_read_slot(&reader);
    if (transaction->recovery_slot == NULL || !relocation_journal_read_token(&reader,"BEGIN\n")) { return false; }
    if (!relocation_recovery_report_shape(&transaction->report) || !relocation_recovery_slot_shape(transaction->recovery_slot,true)) {
        return relocation_journal_invalid(&reader,"The preparation scope or root ownership is invalid");
    }
    transaction->intent_length = (off_t)reader.offset;
    if (transaction->intent_length < 0 || (uintmax_t)transaction->intent_length != reader.offset) {
        return relocation_journal_invalid(&reader,"Preparation intent offset exceeds its supported file range");
    }
    transaction->intent_ready = true;
    return records(&reader,prepared);
}
