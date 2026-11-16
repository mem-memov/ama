/** @file
 * @brief Refuse each actual lease checkpoint while exercising the real filesystem and descriptor cleanup.
 */
/** Counted refusal authority is local to one attempt and never modifies the underlying project. */
typedef struct { size_t count; size_t failure; } Failure;
/** Refuse exactly one ordinal; the real production operation runs for every other checkpoint. */
static bool permit(void *context, RelocationIOOperation operation, const char *physical) {
    Failure *failure = context;
    (void)operation;
    assert(physical != NULL);
    failure->count++;
    return failure->failure == 0 || failure->count != failure->failure;
}
/** A refusal anywhere in a held prefix leaves both sources and later independent attempts usable. */
static void it_refuses_every_measured_lease_io_checkpoint(void) {
    prepare("lease-io");
    const char *recovery = path("recovery1");
    size_t baseline = descriptors();
    RelocationTransaction transaction;
    Failure failure = {0};
    application(&transaction);
    transaction.io = (RelocationIO){&failure,permit};
    assert(relocation_transaction_lease(&transaction,recovery));
    assert(relocation_transaction_revalidate(&transaction));
    size_t measured = failure.count;
    relocation_transaction_destroy(&transaction);
    for (size_t ordinal = 1; ordinal <= measured; ordinal++) {
        application(&transaction);
        failure = (Failure){.failure=ordinal};
        transaction.io = (RelocationIO){&failure,permit};
        bool complete = relocation_transaction_lease(&transaction,recovery) && relocation_transaction_revalidate(&transaction);
        assert(!complete && transaction.status == RELOCATION_IO_ERROR);
        relocation_transaction_destroy(&transaction);
        assert(descriptors() == baseline);
        untouched(&move_request);
    }
    done();
}
