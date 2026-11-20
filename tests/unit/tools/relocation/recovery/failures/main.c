/** @file
 * @brief Fail every measured recovery operation and allocation, then recover again without losing originals or descriptors.
 */
#include <dirent.h>
#include "../published/common.c"

/** Count actual process descriptors independently of production ownership bookkeeping. */
static size_t descriptors(void) {
    DIR *directory = opendir("/proc/self/fd");
    assert(directory != NULL);
    size_t count = 0;
    struct dirent *entry;
    while ((entry = readdir(directory)) != NULL) {
        if (strcmp(entry->d_name,".") != 0 && strcmp(entry->d_name,"..") != 0) { count++; }
    }
    assert(closedir(directory) == 0);
    return count;
}
/** One synchronous refusal at a measured checkpoint; callbacks never mutate workspace state. */
typedef struct { size_t count; size_t refuse; bool failed; } Failure;
/** A refusal is observable exactly once, independently of how recovery handles its failed suffix. */
static bool permit(void *context, RelocationIOOperation operation, const char *path) {
    (void)operation;
    (void)path;
    Failure *failure = context;
    if (++failure->count == failure->refuse) { failure->failed = true; return false; }
    return true;
}
/** Retry uses a fresh owner and disk records alone, including terminal receipts written before a failed acknowledgement. */
static void retry(const char *journal, bool committed) {
    RelocationRecovery recovery;
    relocation_recovery_init(&recovery,sources.allocator);
    bool result = relocation_recovery_run(&recovery,&(RelocationRecoveryRequest){.journal=journal});
    if (!result) { fprintf(stderr,"Retry failed: %s\n",recovery.transaction.error.message); }
    assert(result && recovery.finished && recovery.transaction.committed == committed);
    assert(recovery.rolled_back == !committed);
    importer(committed ? "@!\nvalue\n/test/fixture/v1.0.0/src/new/a\n" : "@!\nvalue\n/test/fixture/v1.0.0/src/old/a\n");
    relocation_recovery_destroy(&recovery);
    if (committed) { complete_fixture(); } else { done(); }
}
/** Prepare a pending publication or an already completed receipt without retaining any recovery executor. */
static const char *published_journal(bool committed, bool terminal) {
    const char *journal = interrupt_publication(committed);
    if (terminal) {
        RelocationRecovery recovery;
        relocation_recovery_init(&recovery,sources.allocator);
        assert(relocation_recovery_run(&recovery,&(RelocationRecoveryRequest){.journal=journal}));
        assert(recovery.finished);
        relocation_recovery_destroy(&recovery);
    }
    return journal;
}
/** Each injected failure must close every acquired descriptor and retain enough disk authority for successful retry. */
static void it_recovers_after_every_filesystem_refusal(void) {
    for (unsigned mode = 0; mode < 4; mode++) {
        bool committed = (mode&1) != 0, terminal = mode >= 2;
        char measurement[80];
        int measurement_length = snprintf(measurement,sizeof(measurement),"io-measure-%u",mode);
        assert(measurement_length > 0 && (size_t)measurement_length < sizeof(measurement));
        prepare(measurement);
        const char *journal = published_journal(committed,terminal);
        RelocationRecovery recovery;
        relocation_recovery_init(&recovery,sources.allocator);
        Failure failure = {0};
        assert(relocation_recovery_run(&recovery,&(RelocationRecoveryRequest){.journal=journal,.io={&failure,permit}}));
        size_t measured = failure.count;
        relocation_recovery_destroy(&recovery);
        if (committed) { complete_fixture(); } else { done(); }
        fprintf(stderr,"recovery %s %s I/O checkpoints: %zu\n",terminal ? "terminal" : "pending",committed ? "commit" : "rollback",measured);
        for (size_t ordinal = 1; ordinal <= measured; ordinal++) {
            char name[80];
            int length = snprintf(name,sizeof(name),"io-%u-%zu",mode,ordinal);
            assert(length > 0 && (size_t)length < sizeof(name));
            prepare(name);
            journal = published_journal(committed,terminal);
            size_t baseline = live_allocations, open_count = descriptors();
            relocation_recovery_init(&recovery,sources.allocator);
            failure = (Failure){.refuse=ordinal};
            assert(!relocation_recovery_run(&recovery,&(RelocationRecoveryRequest){.journal=journal,.io={&failure,permit}}));
            assert(failure.failed && !recovery.finished && recovery.transaction.error.message != NULL);
            relocation_recovery_destroy(&recovery);
            assert(live_allocations == baseline && descriptors() == open_count);
            retry(journal,committed != 0);
        }
    }
}
/** Decode, replacement verification and iterative cleanup failures release every memory prefix before independent retry. */
static void it_recovers_after_every_allocation_refusal(void) {
    for (unsigned mode = 0; mode < 4; mode++) {
        bool committed = (mode&1) != 0, terminal = mode >= 2;
        char measurement[80];
        int measurement_length = snprintf(measurement,sizeof(measurement),"allocation-measure-%u",mode);
        assert(measurement_length > 0 && (size_t)measurement_length < sizeof(measurement));
        prepare(measurement);
        const char *journal = published_journal(committed,terminal);
        RelocationRecovery recovery;
        relocation_recovery_init(&recovery,sources.allocator);
        allocation_count = 0;
        assert(relocation_recovery_run(&recovery,&(RelocationRecoveryRequest){.journal=journal}));
        size_t measured = allocation_count;
        relocation_recovery_destroy(&recovery);
        if (committed) { complete_fixture(); } else { done(); }
        fprintf(stderr,"recovery %s %s allocation checkpoints: %zu\n",terminal ? "terminal" : "pending",committed ? "commit" : "rollback",measured);
        for (size_t ordinal = 1; ordinal <= measured; ordinal++) {
            char name[80];
            int length = snprintf(name,sizeof(name),"allocation-%u-%zu",mode,ordinal);
            assert(length > 0 && (size_t)length < sizeof(name));
            prepare(name);
            journal = published_journal(committed,terminal);
            size_t baseline = live_allocations, open_count = descriptors();
            relocation_recovery_init(&recovery,sources.allocator);
            allocation_count = 0;
            failure_at = ordinal;
            assert(!relocation_recovery_run(&recovery,&(RelocationRecoveryRequest){.journal=journal}));
            failure_at = 0;
            assert(!recovery.finished && recovery.transaction.status == RELOCATION_OUT_OF_MEMORY);
            relocation_recovery_destroy(&recovery);
            assert(live_allocations == baseline && descriptors() == open_count);
            retry(journal,committed != 0);
        }
    }
}
/** Keep failure ownership tests independent of writer interruption acceptance. */
void run(void) {
    it_recovers_after_every_filesystem_refusal();
    it_recovers_after_every_allocation_refusal();
}
/** Exact live counters complement available sanitizers and include temporary record buffers. */
int main(void) { run(); return EXIT_SUCCESS; }
