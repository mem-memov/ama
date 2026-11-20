/** @file
 * @brief Reject changed complete source domains instead of checking only previously listed selected files.
 */
/** Added unused modules, modified importers and removed sources invalidate the plan before any publication. */
static void it_revalidates_complete_scope_and_original_bytes(void) {
    const char *names[] = {"added-source","changed-importer","removed-source"};
    for (size_t partition = 0; partition < 3; partition++) {
        prepare(names[partition]);
        if (partition == 0) { file("imports/packages/test/imports/v1.0.0/src/extra","@!\n"); }
        else if (partition == 1) { file("imports/packages/test/imports/v1.0.0/src/unused","@!\nvalue\n/test/fixture/v1.0.0/src/old/a\n# changed\n"); }
        else { assert(unlink(path("project/packages/test/fixture/v1.0.0/src/old/a")) == 0); }
        RelocationTransaction transaction;
        application(&transaction);
        assert(relocation_transaction_lease(&transaction,path("recovery1")));
        assert(!relocation_transaction_revalidate(&transaction));
        assert(transaction.status == RELOCATION_CHANGED_INPUT);
        assert(transaction.error.primary.path == preview.source);
        relocation_transaction_destroy(&transaction);
        done();
    }
}
