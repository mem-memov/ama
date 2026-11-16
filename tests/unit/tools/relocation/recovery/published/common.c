/** @file
 * @brief Leave real uncommitted or committed publications for independent recovery ownership tests.
 */
#ifndef AMA_RELOCATION_PUBLISHED_TEST_COMMON
#define AMA_RELOCATION_PUBLISHED_TEST_COMMON
#include "../common.c"
#include "tools/relocation/recovery/internal.h"
/** The persisted journal, not the vanished transaction owner, is the sole recovery authority. */
static const char *interrupt_publication(bool committed) {
    folder("recovery");
    RelocationTransaction transaction;
    relocation_transaction_init(&transaction,sources.allocator);
    transaction.staging = &staging;
    assert(relocation_report_build(&transaction.report,&staging));
    assert(relocation_transaction_lease(&transaction,path("recovery")));
    assert(relocation_transaction_prepare(&transaction));
    assert(relocation_transaction_journal(&transaction));
    assert(relocation_transaction_publish(&transaction));
    if (committed) { assert(relocation_transaction_decide(&transaction)); }
    const char *journal = source_arena_copy(&fixture_paths,transaction.journal);
    assert(journal != NULL);
    relocation_transaction_destroy(&transaction);
    return journal;
}
#endif
