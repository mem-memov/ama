/** @file
 * @brief Recover real process exits at every durable preparation, publication and cleanup checkpoint.
 */
#include <dirent.h>
#include <sys/wait.h>
#include "../published/common.c"
#include "tools/relocation/recovery/recovery.h"

/** Count effects only after the complete initial preparation authority exists; before it no nonempty private copy is possible. */
typedef struct {
    RelocationTransaction *transaction;
    size_t count;
    size_t interrupt_at;
} Interruption;
/** Actual process termination skips every in-memory destructor and inverse, matching a vanished application executor. */
static bool permit(void *context, RelocationIOOperation operation, const char *path) {
    (void)operation;
    (void)path;
    Interruption *interruption = context;
    if (interruption->transaction->intent_ready && ++interruption->count == interruption->interrupt_at) { _exit(73); }
    return true;
}
/** Execute the complete real application with a phase-aware observation/termination adapter. */
static size_t apply(size_t interrupt_at) {
    RelocationTransaction transaction;
    relocation_transaction_init(&transaction,sources.allocator);
    Interruption interruption = {.transaction=&transaction,.interrupt_at=interrupt_at};
    RelocationTransactionRequest request = {.staging=&staging,.recovery_directory=path("recovery"),.io={&interruption,permit}};
    assert(relocation_transaction_apply(&transaction,&request));
    relocation_transaction_destroy(&transaction);
    return interruption.count;
}
/** The isolated existing recovery directory contains exactly the single journal created by its child process. */
static const char *journal(void) {
    DIR *directory = opendir(path("recovery"));
    assert(directory != NULL);
    const char *result = NULL;
    struct dirent *entry;
    while ((entry = readdir(directory)) != NULL) {
        if (strcmp(entry->d_name,".") == 0 || strcmp(entry->d_name,"..") == 0) { continue; }
        assert(result == NULL && strncmp(entry->d_name,".ama-relocation-",16) == 0);
        result = source_arena_join(&fixture_paths,path("recovery"),"/",entry->d_name);
        assert(result != NULL);
    }
    assert(closedir(directory) == 0 && result != NULL);
    return result;
}
/** Every completed record/copy/rename/cleanup prefix must recover through newly decoded disk authority alone. */
static void it_recovers_every_registered_process_interruption(void) {
    prepare("interruption-measurement");
    folder("recovery");
    size_t measured = apply(0);
    assert(measured != 0);
    complete_fixture();
    fprintf(stderr,"registered process interruption checkpoints: %zu\n",measured);
    for (size_t ordinal = 1; ordinal <= measured; ordinal++) {
        char name[80];
        int length = snprintf(name,sizeof(name),"interruption-%zu",ordinal);
        assert(length > 0 && (size_t)length < sizeof(name));
        prepare(name);
        folder("recovery");
        pid_t child = fork();
        assert(child >= 0);
        if (child == 0) { (void)apply(ordinal); _exit(0); }
        int status;
        assert(waitpid(child,&status,0) == child && WIFEXITED(status) && WEXITSTATUS(status) == 73);
        const char *record = journal();
        RelocationRecovery recovery;
        relocation_recovery_init(&recovery,sources.allocator);
        bool recovered = relocation_recovery_run(&recovery,&(RelocationRecoveryRequest){.journal=record});
        if (!recovered) {
            fprintf(stderr,"Interruption %zu/%zu failed recovery: %s\n",ordinal,measured,recovery.transaction.error.message);
        }
        assert(recovered && recovery.ready && recovery.finished);
        bool committed = recovery.transaction.committed;
        assert(recovery.rolled_back == !committed);
        importer(committed ? "@!\nvalue\n/test/fixture/v1.0.0/src/new/a\n" : "@!\nvalue\n/test/fixture/v1.0.0/src/old/a\n");
        struct stat metadata;
        assert(lstat(path(committed ? "project/packages/test/fixture/v1.0.0/src/new/a" : "project/packages/test/fixture/v1.0.0/src/old/a"),&metadata) == 0);
        assert(lstat(path(committed ? "project/packages/test/fixture/v1.0.0/src/old" : "project/packages/test/fixture/v1.0.0/src/new"),&metadata) != 0 && errno == ENOENT);
        relocation_recovery_destroy(&recovery);
        relocation_recovery_init(&recovery,sources.allocator);
        assert(relocation_recovery_run(&recovery,&(RelocationRecoveryRequest){.journal=record}));
        assert(recovery.finished && recovery.transaction.committed == committed);
        relocation_recovery_destroy(&recovery);
        if (committed) { complete_fixture(); } else { done(); }
    }
}
/** Recovery interruption must abandon executor memory and descriptors just as the original writer interruption does. */
typedef struct { size_t count; size_t interrupt_at; } RecoveryInterruption;
/** Every requested operation is measured, including initial loading and final outcome acknowledgement. */
static bool recovery_permit(void *context, RelocationIOOperation operation, const char *path) {
    (void)operation;
    (void)path;
    RecoveryInterruption *interruption = context;
    if (++interruption->count == interruption->interrupt_at) { _exit(74); }
    return true;
}
/** Repeated interruptions during inverses and retirement must remain recoverable from unchanged persisted authority. */
static void it_recovers_every_interrupted_recovery_process(void) {
    for (unsigned committed = 0; committed < 2; committed++) {
        prepare(committed ? "recovery-interruption-commit" : "recovery-interruption-rollback");
        const char *record = interrupt_publication(committed != 0);
        RecoveryInterruption interruption = {0};
        RelocationRecovery recovery;
        relocation_recovery_init(&recovery,sources.allocator);
        assert(relocation_recovery_run(&recovery,&(RelocationRecoveryRequest){.journal=record,.io={&interruption,recovery_permit}}));
        size_t measured = interruption.count;
        relocation_recovery_destroy(&recovery);
        if (committed) { complete_fixture(); } else { done(); }
        fprintf(stderr,"recovery %s process interruption checkpoints: %zu\n",committed ? "commit" : "rollback",measured);
        for (size_t ordinal = 1; ordinal <= measured; ordinal++) {
            char name[96];
            int length = snprintf(name,sizeof(name),"recovery-interruption-%u-%zu",committed,ordinal);
            assert(length > 0 && (size_t)length < sizeof(name));
            prepare(name);
            record = interrupt_publication(committed != 0);
            pid_t child = fork();
            assert(child >= 0);
            if (child == 0) {
                relocation_recovery_init(&recovery,sources.allocator);
                interruption = (RecoveryInterruption){.interrupt_at=ordinal};
                bool result = relocation_recovery_run(&recovery,&(RelocationRecoveryRequest){.journal=record,.io={&interruption,recovery_permit}});
                _exit(result ? 0 : 1);
            }
            int status;
            assert(waitpid(child,&status,0) == child && WIFEXITED(status) && WEXITSTATUS(status) == 74);
            relocation_recovery_init(&recovery,sources.allocator);
            bool result = relocation_recovery_run(&recovery,&(RelocationRecoveryRequest){.journal=record});
            if (!result) { fprintf(stderr,"Interrupted recovery %u:%zu/%zu failed: %s\n",committed,ordinal,measured,recovery.transaction.error.message); }
            assert(result && recovery.finished && recovery.transaction.committed == (committed != 0));
            importer(committed ? "@!\nvalue\n/test/fixture/v1.0.0/src/new/a\n" : "@!\nvalue\n/test/fixture/v1.0.0/src/old/a\n");
            relocation_recovery_destroy(&recovery);
            relocation_recovery_init(&recovery,sources.allocator);
            assert(relocation_recovery_run(&recovery,&(RelocationRecoveryRequest){.journal=record}));
            assert(recovery.finished && recovery.rolled_back == !committed);
            relocation_recovery_destroy(&recovery);
            if (committed) { complete_fixture(); } else { done(); }
        }
    }
}

/** Every selected checkpoint comes from a measured successful execution, without hardcoded operation counts. */
void run(void) {
    it_recovers_every_registered_process_interruption();
    it_recovers_every_interrupted_recovery_process();
}
/** The real fork/exit boundary intentionally abandons child allocations; the parent retains exact live-owner assertions. */
int main(void) { run(); return EXIT_SUCCESS; }
