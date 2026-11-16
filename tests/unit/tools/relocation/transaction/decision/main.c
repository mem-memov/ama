/** @file
 * @brief Refuse commit acknowledgement and its inverse independently without guessing whether source rollback is safe.
 */
#include "../preflight/common.c"

/** Phase-aware fault authority observes the actual transaction rather than depending on an incidental syscall count. */
typedef struct {
    RelocationTransaction *transaction;
    bool commit_started;
    bool primary_failed;
    bool deny_retraction;
    size_t inverses;
} DecisionFailure;
/** Refuse the first commit record sync; optionally refuse its retraction while allowing every unrelated real filesystem operation. */
static bool permit(void *context, RelocationIOOperation operation, const char *physical) {
    DecisionFailure *failure = context;
    RelocationTransaction *transaction = failure->transaction;
    if (operation == RELOCATION_IO_RESTORE) { failure->inverses++; }
    if (operation == RELOCATION_IO_JOURNAL && transaction->state == RELOCATION_TRANSACTION_APPLYING) {
        if (!transaction->cleaning) { failure->commit_started = true; }
        else if (failure->deny_retraction) { return false; }
    }
    if (operation == RELOCATION_IO_SYNC && failure->commit_started && !failure->primary_failed
        && !transaction->cleaning && strcmp(physical,transaction->journal_record) == 0) {
        failure->primary_failed = true;
        return false;
    }
    return true;
}
/** Completed durable retraction permits ordinary inverse restoration and removes all temporary recovery data. */
static void it_retracts_a_failed_commit_before_restoring_sources(void) {
    prepare("retracted-decision");
    size_t baseline = descriptors();
    RelocationTransaction transaction;
    application(&transaction);
    DecisionFailure failure = {.transaction=&transaction};
    RelocationTransactionRequest request = {.staging=&staging,.recovery_directory=path("recovery1"),.io={&failure,permit}};
    assert(!relocation_transaction_apply(&transaction,&request));
    assert(failure.primary_failed && failure.inverses != 0);
    assert(!transaction.committed && !transaction.decision_pending && transaction.state == RELOCATION_TRANSACTION_ROLLED_BACK);
    assert(transaction.error.message != NULL);
    struct stat metadata;
    assert(lstat(transaction.journal,&metadata) != 0 && errno == ENOENT);
    relocation_transaction_destroy(&transaction);
    assert(descriptors() == baseline);
    done();
}
/** A potentially present commit record forbids all inverse publication until explicit recovery can resolve the decision. */
static void it_retains_both_states_when_commit_retraction_fails(void) {
    prepare("uncertain-decision");
    size_t baseline = descriptors();
    RelocationTransaction transaction;
    application(&transaction);
    DecisionFailure failure = {.transaction=&transaction,.deny_retraction=true};
    RelocationTransactionRequest request = {.staging=&staging,.recovery_directory=path("recovery1"),.io={&failure,permit}};
    assert(!relocation_transaction_apply(&transaction,&request));
    assert(failure.primary_failed && failure.inverses == 0);
    assert(!transaction.committed && transaction.decision_pending && transaction.state == RELOCATION_TRANSACTION_INCOMPLETE);
    assert(transaction.error.message != NULL && transaction.cleanup_error.message != NULL);
    struct stat metadata;
    assert(lstat(preview.source,&metadata) != 0 && errno == ENOENT);
    assert(lstat(preview.destination,&metadata) == 0 && S_ISDIR(metadata.st_mode));
    assert(lstat(transaction.journal,&metadata) == 0);
    const SourceText *record = NULL;
    assert(source_manager_load(&sources,transaction.journal_record,&record) == SOURCE_OK);
    size_t length = source_length(record);
    assert(length >= 7 && memcmp(source_bytes(record)+length-7,"COMMIT\n",7) == 0);
    for (const RelocationTransactionAction *action = transaction.first; action != NULL; action = action->next) {
        if (action->kind != RELOCATION_ACTION_CREATE) {
            assert(lstat(action->slot->backup,&metadata) == 0);
            assert(metadata.st_dev == action->original_metadata.st_dev && metadata.st_ino == action->original_metadata.st_ino);
        }
    }
    relocation_transaction_destroy(&transaction);
    assert(descriptors() == baseline);
    /* The test retains disk evidence until the successful leaf harness removes its
     * wholly owned fixture; no automatic interruption replay is claimed here. */
    relocation_staging_destroy(&staging);
    relocation_references_destroy(&references);
    reset_preview();
    finish();
}
/** Exercise acknowledgement failure with and without a safely established inverse decision. */
void run(void) {
    it_retracts_a_failed_commit_before_restoring_sources();
    it_retains_both_states_when_commit_retraction_fails();
}
/** Disk and descriptor assertions precede fixture retirement. */
int main(void) { run(); return EXIT_SUCCESS; }
