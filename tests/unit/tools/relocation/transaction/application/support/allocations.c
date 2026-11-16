/** @file
 * @brief Fail every measured application allocation across locks, copies, journals, inverses and committed cleanup.
 */
/** A precommit allocation refusal restores originals; a postcommit cleanup refusal retains the committed journal and new project. */
static void it_unwinds_every_application_allocation_without_losing_originals(void) {
    failure_project("allocation-measure");
    RelocationTransaction transaction;
    application(&transaction);
    RelocationTransactionRequest request = {.staging=&staging,.recovery_directory=path("recovery1")};
    allocation_count = 0;
    assert(relocation_transaction_apply(&transaction,&request));
    size_t measured = allocation_count;
    relocation_transaction_destroy(&transaction);
    finish_applied();
    fprintf(stderr,"application allocations: %zu\n",measured);
    for (size_t ordinal = 1; ordinal <= measured; ordinal++) {
        char name[80];
        int length = snprintf(name,sizeof(name),"allocation-failure-%zu",ordinal);
        assert(length > 0 && (size_t)length < sizeof(name));
        failure_project(name);
        size_t baseline = live_allocations, descriptor_baseline = descriptors();
        application(&transaction);
        request = (RelocationTransactionRequest){.staging=&staging,.recovery_directory=path("recovery1")};
        allocation_count = 0;
        failure_at = ordinal;
        bool applied = relocation_transaction_apply(&transaction,&request);
        failure_at = 0;
        if (applied || transaction.error.code != DIAGNOSTIC_OUT_OF_MEMORY) {
            fprintf(stderr,"Application allocation %zu/%zu: applied=%d status=%d state=%d cause=%d (%s)\n",
                ordinal,measured,applied,transaction.status,transaction.state,transaction.error.code,
                transaction.error.message == NULL ? "none" : transaction.error.message);
        }
        assert(!applied && transaction.error.code == DIAGNOSTIC_OUT_OF_MEMORY);
        bool was_committed = transaction.committed;
        if (was_committed) {
            assert(transaction.state == RELOCATION_TRANSACTION_INCOMPLETE);
            struct stat metadata;
            assert(lstat(transaction.journal,&metadata) == 0);
            transaction.cleaning = true;
            assert(relocation_transaction_cleanup(&transaction));
            transaction.cleaning = false;
        } else {
            assert(transaction.state == RELOCATION_TRANSACTION_ROLLED_BACK && !transaction.decision_pending);
        }
        assert(descriptors() == descriptor_baseline);
        relocation_transaction_destroy(&transaction);
        assert(live_allocations == baseline);
        if (was_committed) { committed(); finish_applied(); }
        else { restored(); done(); }
    }
}
