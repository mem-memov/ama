/** @file
 * @brief Own exclusive relocation application, retained originals and rollback state.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires complete preparation, unchanged originals and recoverable publication.
 */
#ifndef AMA_RELOCATION_TRANSACTION_H
#define AMA_RELOCATION_TRANSACTION_H
#include "../staging/staging.h"
#include "../report/report.h"

/** Observable filesystem phase for deterministic fault injection; callbacks never execute Ama or reenter application. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
typedef enum {
    RELOCATION_IO_LEASE, RELOCATION_IO_REVALIDATE, RELOCATION_IO_TEMPORARY,
    RELOCATION_IO_DIRECTORY, RELOCATION_IO_FILE, RELOCATION_IO_WRITE,
    RELOCATION_IO_CLOSE, RELOCATION_IO_MODE, RELOCATION_IO_OWNER, RELOCATION_IO_TIMES, RELOCATION_IO_VERIFY,
    RELOCATION_IO_LINK, RELOCATION_IO_SYNC, RELOCATION_IO_JOURNAL,
    RELOCATION_IO_BACKUP, RELOCATION_IO_PUBLISH, RELOCATION_IO_RESTORE,
    RELOCATION_IO_REMOVE, RELOCATION_IO_READ
} RelocationIOOperation;
/** Optional synchronous fault adapter; false refuses the named operation before its filesystem effect.
 * Close checkpoints occur after the descriptor's sole actual close attempt so refusal
 * never loses descriptor ownership. The callback/context survive apply, do not mutate
 * project state and must not reenter the transaction. Null permits actual OS operations.
 * Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
typedef struct {
    void *context;
    bool (*permit)(void *, RelocationIOOperation, const char *);
} RelocationIO;
/** Application outcome distinguishes source rollback from committed cleanup still requiring recovery. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
typedef enum {
    RELOCATION_TRANSACTION_EMPTY, RELOCATION_TRANSACTION_PREPARING,
    RELOCATION_TRANSACTION_PREPARED, RELOCATION_TRANSACTION_APPLYING,
    RELOCATION_TRANSACTION_COMMITTED, RELOCATION_TRANSACTION_ROLLED_BACK,
    RELOCATION_TRANSACTION_INCOMPLETE
} RelocationTransactionState;
/** Borrow the complete plan and an existing recovery directory outside the moved tree.
 * Cooperating filesystem writers must honor the same directory leases; arbitrary
 * noncooperating writers are outside exclusive-access preconditions. The plan and
 * allocator authority survive transaction destruction. Recovery data survives failure.
 * Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
typedef struct {
    const RelocationStaging *staging;
    const char *recovery_directory;
    RelocationIO io;
} RelocationTransactionRequest;
/** Private owned directory descriptors remain locked through the complete application attempt. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
typedef struct RelocationDirectoryLease RelocationDirectoryLease;
/** Private same-filesystem staging/backup slots own only explicitly created paths. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
typedef struct RelocationTransactionSlot RelocationTransactionSlot;
/** Private ordered changes retain their inverse before publication begins. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
typedef struct RelocationTransactionAction RelocationTransactionAction;
/** Linear application owner. Destruction releases descriptors and memory, never silently
 * retries incomplete rollback or deletes retained recovery data. Paths/diagnostics survive
 * the attempt until destruction. committed distinguishes irrevocable commit from rollback.
 * Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
typedef struct {
    const RelocationStaging *staging;
    SourceArena arena;
    RelocationReport report;
    RelocationIO io;
    RelocationDirectoryLease *leases;
    RelocationTransactionSlot *slots;
    RelocationTransactionSlot *recovery_slot;
    RelocationTransactionAction *first;
    RelocationTransactionAction *last;
    const char *journal;
    const char *journal_record;
    const char *intent_record;
    size_t slot_ordinal;
    off_t journal_length;
    off_t intent_length;
    const char *recovery_directory;
    RelocationStatus status;
    Diagnostic error;
    Diagnostic cleanup_error;
    RelocationTransactionState state;
    bool attempted;
    bool retain_receipt;
    bool intent_ready;
    bool committed;
    bool decision_pending;
    bool cleaning;
} RelocationTransaction;
/** Initialize memory ownership without acquiring locks or creating files. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
void relocation_transaction_init(RelocationTransaction *, SourceAllocator);
/** Release descriptor leases and memory; retained incomplete recovery files remain on disk. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
void relocation_transaction_destroy(RelocationTransaction *);
/** Apply one validated plan, rolling back ordinary precommit failures and retaining incomplete recovery data. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_transaction_apply(RelocationTransaction *, const RelocationTransactionRequest *);
#endif
