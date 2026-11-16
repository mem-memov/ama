/** @file
 * @brief Decode real prepared inventories, reject all truncations and exhaust ownership failures.
 */
#include <inttypes.h>
#include "../../report/common.c"
#include "tools/relocation/recovery/internal.h"

/** Produce actual bytes through the production writer while source paths remain unchanged. */
static const SourceText *journal(RelocationTransaction *transaction) {
    folder("recovery");
    relocation_transaction_init(transaction,sources.allocator);
    transaction->staging = &staging;
    assert(relocation_report_build(&transaction->report,&staging));
    assert(relocation_transaction_lease(transaction,path("recovery")));
    assert(relocation_transaction_prepare(transaction));
    assert(relocation_transaction_journal(transaction));
    const SourceText *record;
    assert(source_manager_load(&sources,transaction->journal_record,&record) == SOURCE_OK);
    return record;
}
/** Valid frames keep Unicode/raw bytes and independent source/report owners; malformed integers never coerce. */
static void it_decodes_canonical_scalar_frames_exactly(void) {
    setup_test();
    const char *invalid[] = {"", "-0\n", "+1\n", "01\n", "1 \n", "1\r\n", "184467440737095516160\n", "-92233720368547758090\n"};
    for (size_t index = 0; index < sizeof(invalid)/sizeof(*invalid); index++) {
        RelocationRecovery recovery;
        relocation_recovery_init(&recovery,sources.allocator);
        const SourceText *text;
        assert(source_manager_add(&sources,"scalar",(const unsigned char *)invalid[index],strlen(invalid[index]),&text) == SOURCE_OK);
        RelocationJournalReader reader = {.recovery=&recovery,.source=text};
        intmax_t result;
        assert(!relocation_journal_read_signed(&reader,&result));
        assert(recovery.transaction.status == RELOCATION_INVALID_JOURNAL);
        relocation_recovery_destroy(&recovery);
    }
    char extremes[128];
    int length = snprintf(extremes,sizeof(extremes),"%" PRIdMAX "\n%" PRIdMAX "\n0\n1\n",INTMAX_MIN,INTMAX_MAX);
    assert(length > 0 && (size_t)length < sizeof(extremes));
    const SourceText *text;
    assert(source_manager_add(&sources,"extremes",(const unsigned char *)extremes,(size_t)length,&text) == SOURCE_OK);
    RelocationRecovery recovery;
    relocation_recovery_init(&recovery,sources.allocator);
    RelocationJournalReader reader = {.recovery=&recovery,.source=text};
    intmax_t integer;
    bool boolean;
    assert(relocation_journal_read_signed(&reader,&integer) && integer == INTMAX_MIN);
    assert(relocation_journal_read_signed(&reader,&integer) && integer == INTMAX_MAX);
    assert(relocation_journal_read_boolean(&reader,&boolean) && !boolean);
    assert(relocation_journal_read_boolean(&reader,&boolean) && boolean);
    assert(reader.offset == source_length(text));
    relocation_recovery_destroy(&recovery);
    destroy_test();
}
/** A complete original inventory round-trips; every missing prepared byte rejects before publication. */
static void it_rejects_every_truncated_inventory_prefix(void) {
    prepare("decode-truncation");
    RelocationTransaction transaction;
    const SourceText *record = journal(&transaction);
    RelocationRecovery recovery;
    relocation_recovery_init(&recovery,sources.allocator);
    assert(relocation_journal_decode(&recovery,record));
    assert(relocation_journal_validate(&recovery));
    assert(!recovery.finished && !recovery.transaction.committed);
    assert(strcmp(recovery.transaction.report.source,transaction.report.source) == 0);
    assert(recovery.transaction.report.before_root_count == transaction.report.before_root_count);
    assert(recovery.transaction.first->next->tree_count == transaction.first->next->tree_count);
    relocation_recovery_destroy(&recovery);
    size_t baseline = live_allocations;
    for (size_t length = 0; length < source_length(record); length++) {
        SourceManager truncated;
        source_manager_init(&truncated,sources.allocator);
        const SourceText *prefix;
        assert(source_manager_add(&truncated,"truncated",source_bytes(record),length,&prefix) == SOURCE_OK);
        relocation_recovery_init(&recovery,sources.allocator);
        assert(!relocation_journal_decode(&recovery,prefix));
        assert(recovery.transaction.status == RELOCATION_INVALID_JOURNAL && !recovery.ready);
        relocation_recovery_destroy(&recovery);
        source_manager_destroy(&truncated);
        assert(live_allocations == baseline);
    }
    assert(relocation_transaction_cleanup(&transaction));
    relocation_transaction_destroy(&transaction);
    done();
}
/** Every allocation failure releases its prefix and keeps physical originals untouched. */
static void it_releases_every_failed_decode_allocation(void) {
    prepare("decode-allocations");
    RelocationTransaction transaction;
    const SourceText *record = journal(&transaction);
    RelocationRecovery recovery;
    size_t baseline = live_allocations;
    relocation_recovery_init(&recovery,sources.allocator);
    allocation_count = 0;
    assert(relocation_journal_decode(&recovery,record));
    size_t measured = allocation_count;
    relocation_recovery_destroy(&recovery);
    for (size_t ordinal = 1; ordinal <= measured; ordinal++) {
        relocation_recovery_init(&recovery,sources.allocator);
        allocation_count = 0;
        failure_at = ordinal;
        assert(!relocation_journal_decode(&recovery,record));
        failure_at = 0;
        assert(recovery.transaction.status == RELOCATION_OUT_OF_MEMORY);
        relocation_recovery_destroy(&recovery);
        assert(live_allocations == baseline);
    }
    assert(relocation_transaction_cleanup(&transaction));
    relocation_transaction_destroy(&transaction);
    done();
}
/** Decision/finalization tokens accept exact records or proper incomplete suffixes, never arbitrary trailing data. */
static void it_decodes_only_exact_decision_and_terminal_tails(void) {
    prepare("decode-tails");
    RelocationTransaction transaction;
    const SourceText *record = journal(&transaction);
    const char *valid[] = {"", "C", "COMMIT", "COMMIT\n", "COMMIT\nD", "COMMIT\nDONE-COMMIT\n", "D", "DONE-ROLLBACK\n"};
    const char *invalid[] = {"COMMIT\r\n", "COMMIT\nDONE-ROLLBACK\n", "DONE-COMMIT\n", "DONE-ROLLBACK\nextra", "x"};
    for (unsigned group = 0; group < 2; group++) {
        const char *const *tails = group == 0 ? valid : invalid;
        size_t count = group == 0 ? sizeof(valid)/sizeof(*valid) : sizeof(invalid)/sizeof(*invalid);
        for (size_t index = 0; index < count; index++) {
            const char *tail = tails[index];
            size_t length = source_length(record)+strlen(tail);
            unsigned char *buffer = malloc(length);
            assert(buffer != NULL);
            memcpy(buffer,source_bytes(record),source_length(record));
            memcpy(buffer+source_length(record),tail,strlen(tail));
            SourceManager manager;
            source_manager_init(&manager,sources.allocator);
            const SourceText *text;
            assert(source_manager_add(&manager,"tail",buffer,length,&text) == SOURCE_OK);
            free(buffer);
            RelocationRecovery recovery;
            relocation_recovery_init(&recovery,sources.allocator);
            bool accepted = relocation_journal_decode(&recovery,text);
            assert(accepted == (group == 0));
            if (accepted) {
                assert(recovery.transaction.committed == (strncmp(tail,"COMMIT\n",7) == 0));
                assert(recovery.finished == (strcmp(tail,"COMMIT\nDONE-COMMIT\n") == 0 || strcmp(tail,"DONE-ROLLBACK\n") == 0));
            } else { assert(recovery.transaction.status == RELOCATION_INVALID_JOURNAL); }
            relocation_recovery_destroy(&recovery);
            source_manager_destroy(&manager);
        }
    }
    assert(relocation_transaction_cleanup(&transaction));
    relocation_transaction_destroy(&transaction);
    done();
}
/** Text rejects embedded NUL while binary snapshots preserve it; invalid timestamp fractions cannot become identities. */
static void it_distinguishes_binary_payloads_and_metadata_ranges(void) {
    setup_test();
    const unsigned char payload[] = {'3','\n','x',0,'y'};
    const SourceText *text;
    assert(source_manager_add(&sources,"frames",payload,sizeof(payload),&text) == SOURCE_OK);
    RelocationRecovery recovery;
    relocation_recovery_init(&recovery,sources.allocator);
    RelocationJournalReader reader = {.recovery=&recovery,.source=text};
    const char *string;
    assert(!relocation_journal_read_text(&reader,&recovery.transaction.arena,&string));
    assert(recovery.transaction.status == RELOCATION_INVALID_JOURNAL);
    relocation_recovery_destroy(&recovery);
    relocation_recovery_init(&recovery,sources.allocator);
    reader = (RelocationJournalReader){.recovery=&recovery,.source=text};
    const SourceText *blob;
    assert(relocation_journal_read_blob(&reader,"blob",&blob));
    assert(source_length(blob) == 3 && memcmp(source_bytes(blob),payload+2,3) == 0);
    relocation_recovery_destroy(&recovery);
    const char *metadata = "0\n0\n0\n0\n0\n0\n0\n0\n0\n0\n0\n0\n1000000000\n";
    assert(source_manager_add(&sources,"metadata",(const unsigned char *)metadata,strlen(metadata),&text) == SOURCE_OK);
    relocation_recovery_init(&recovery,sources.allocator);
    reader = (RelocationJournalReader){.recovery=&recovery,.source=text};
    struct stat fields;
    assert(!relocation_journal_read_metadata(&reader,&fields));
    assert(recovery.transaction.status == RELOCATION_INVALID_JOURNAL);
    relocation_recovery_destroy(&recovery);
    destroy_test();
}
/** Use independent leaf execution to cover decoding without relying on recovery to validate its own output. */
void run(void) {
    it_decodes_canonical_scalar_frames_exactly();
    it_rejects_every_truncated_inventory_prefix();
    it_releases_every_failed_decode_allocation();
    it_decodes_only_exact_decision_and_terminal_tails();
    it_distinguishes_binary_payloads_and_metadata_ranges();
}
/** Assertions remain enabled in this leaf even when production compilation defines NDEBUG. */
int main(void) { run(); return EXIT_SUCCESS; }
