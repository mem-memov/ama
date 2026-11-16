/** @file
 * @brief Separate lease acquisition, revalidation, preparation, publication and inverse cleanup.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * preserves originals until commit and keeps failed recovery data accessible.
 */
#ifndef AMA_RELOCATION_TRANSACTION_INTERNAL_H
#define AMA_RELOCATION_TRANSACTION_INTERNAL_H
#include "transaction.h"
/** Directory identity and held descriptor; repeated physical identities share one lease. */
struct RelocationDirectoryLease {
    RelocationDirectoryLease *next;
    int descriptor;
    dev_t device;
    ino_t inode;
    const char *path;
};
/** One exclusively created sibling directory; paths are owned even after partial preparation failure. */
struct RelocationTransactionSlot {
    RelocationTransactionSlot *next;
    const char *directory;
    const char *parent;
    const char *prepared;
    const char *backup;
    struct stat metadata;
    bool created;
    bool acknowledged;
};
/** One physical entry of the independently prepared destination tree. */
typedef struct {
    const RelocationStagedFile *file;
    const char *path;
    struct stat metadata;
    bool created;
} RelocationPreparedEntry;
/** An inverse exists before its forward step can publish any source change. */
typedef enum { RELOCATION_ACTION_REMOVE, RELOCATION_ACTION_CREATE, RELOCATION_ACTION_REPLACE } RelocationActionKind;
/** Ordered forward/inverse state, retaining physical originals until the commit decision. */
struct RelocationTransactionAction {
    RelocationTransactionAction *next;
    RelocationTransactionAction *previous;
    RelocationActionKind kind;
    const char *path;
    RelocationTransactionSlot *slot;
    const RelocationStagedFile *file;
    RelocationPreparedEntry *tree;
    size_t tree_count;
    struct stat original_metadata;
    struct stat prepared_metadata;
    bool saved;
    bool published;
    bool backup_attempted;
    bool publish_attempted;
};
/** Allocate all forward/inverse actions and prepare their private replacement objects before publication. */
bool relocation_transaction_prepare(RelocationTransaction *);
/** Copy the complete selected physical tree into its destination-filesystem slot. */
bool relocation_transaction_tree(RelocationTransaction *, RelocationTransactionAction *);
/** Check whether an explicitly invalidated output contains a projected physical entry. */
bool relocation_transaction_invalidated(const RelocationStaging *, const char *);
/** Allocate/link one ordered action before creating any of its private storage. */
RelocationTransactionAction *relocation_transaction_action(RelocationTransaction *, RelocationActionKind, const char *);
/** Synchronize a verified directory and close its descriptor once. */
bool relocation_transaction_sync(RelocationTransaction *, const char *);
/** Persist the preparation scope before any non-journal slot or copied data exists. */
bool relocation_transaction_intent(RelocationTransaction *);
/** Reserve one exact slot name durably before directory creation. */
bool relocation_transaction_reserve(RelocationTransaction *, const RelocationTransactionSlot *);
/** Record the newly created slot identity durably before any data is copied into it. */
bool relocation_transaction_own(RelocationTransaction *, const RelocationTransactionSlot *);
/** Mark the full prepared plan durable before publication may begin. */
bool relocation_transaction_ready(RelocationTransaction *);
/** Persist preparation abort after all registered private copies retire. */
bool relocation_transaction_abandoned(RelocationTransaction *);
/** Persist the complete prepared action/path inventory before any source mutation. */
bool relocation_transaction_journal(RelocationTransaction *);
/** Flush published parent entries and commit the journal; failed provisional records are durably retracted before rollback. */
bool relocation_transaction_decide(RelocationTransaction *);
/** Persist the completed commit/rollback outcome after all non-receipt private slots retire. */
bool relocation_transaction_finish(RelocationTransaction *);
/** Write all bytes to an owned descriptor, checking short writes and interruption exactly. */
bool relocation_transaction_write(RelocationTransaction *, int, const char *, const unsigned char *, size_t);
/** Apply every prepared forward action, recording inverse intent before each rename. */
bool relocation_transaction_publish(RelocationTransaction *);
/** Restore every attempted action in reverse order, preserving failed originals and primary diagnostics. */
bool relocation_transaction_rollback(RelocationTransaction *);
/** Synchronize restored public parents before private copies retire; actions with no attempted mutation need no flush. */
bool relocation_transaction_restored(RelocationTransaction *);
/** Probe a path as absent, expected identity, other identity, or failed inspection. */
typedef enum { RELOCATION_PATH_ABSENT, RELOCATION_PATH_EXPECTED, RELOCATION_PATH_OTHER, RELOCATION_PATH_ERROR } RelocationPathState;
RelocationPathState relocation_transaction_probe(RelocationTransaction *, const char *, const struct stat *);
/** Verify captured original attributes and regular bytes immediately before backup. */
bool relocation_transaction_original(RelocationTransaction *, const RelocationTransactionAction *);
/** Remove one proven-owned private tree iteratively without following symlink targets. */
bool relocation_transaction_remove(RelocationTransaction *, const char *);
/** Retire owned slots, retaining committed receipts and any journal whose ordinary cleanup remains incomplete. */
bool relocation_transaction_cleanup(RelocationTransaction *);
/** Create and retain one private sibling slot without replacing an existing path. */
RelocationTransactionSlot *relocation_transaction_slot(RelocationTransaction *, const char *);
/** Choose an absent private sibling name bound to this journal identity and reserve its ordinal exactly. */
const char *relocation_transaction_slot_name(RelocationTransaction *, const char *);
/** Atomically rename into an absent target; supported Linux filesystems must implement no-replace rename. */
bool relocation_transaction_rename(RelocationTransaction *, RelocationIOOperation, const char *, const char *);
/** Write/copy one exact file into an absent private path and preserve its captured attributes. */
bool relocation_transaction_file(RelocationTransaction *, const char *, const SourceText *, const struct stat *, struct stat *);
/** Finalize an existing private directory's owner/mode/times, then sync and capture its identity. */
bool relocation_transaction_directory(RelocationTransaction *, const char *, const struct stat *, struct stat *);
/** Create and verify a literal symbolic link without following its target. */
bool relocation_transaction_link(RelocationTransaction *, const char *, const SourceText *, const struct stat *, struct stat *);
/** Apply captured ownership, mode and timestamps to one owned descriptor, then synchronize it. */
bool relocation_transaction_attributes(RelocationTransaction *, int, const char *, const struct stat *);
/** Compare a seekable owned file descriptor against exact expected bytes using bounded stack storage. */
bool relocation_transaction_verify(RelocationTransaction *, int, const char *, const SourceText *);
/** Close an owned descriptor exactly once, recording actual/injected errors after close. */
bool relocation_transaction_close(RelocationTransaction *, int, const char *);
/** Preserve primary failure; cleanup failures use the separate secondary diagnostic. */
bool relocation_transaction_fail(RelocationTransaction *, RelocationStatus, const char *, const char *);
/** Admit a named operation or record a deterministic injected refusal without filesystem effects. */
bool relocation_transaction_permit(RelocationTransaction *, RelocationIOOperation, const char *);
/** Acquire exclusive nonblocking directory leases for all configured roots/endpoints/external input parents. */
bool relocation_transaction_lease(RelocationTransaction *, const char *);
/** Acquire or reuse one already verified existing directory by physical identity. */
bool relocation_transaction_lease_directory(RelocationTransaction *, const char *);
/** Acquire a canonical path's existing parent directory lease. */
bool relocation_transaction_lease_parent(RelocationTransaction *, const char *);
/** Rebuild the complete read-only plan under directory leases and reject any changed scope or bytes. */
bool relocation_transaction_revalidate(RelocationTransaction *);
/** Compare complete original/final file and identity records with an independently rebuilt staging plan. */
bool relocation_transaction_same_plan(RelocationTransaction *, const RelocationStaging *);
/** Reconstruct the explicit preparation request from retained canonical plan records. */
bool relocation_transaction_request(RelocationTransaction *, RelocationRequest *, RelocationStagingRequest *);
/** Close each acquired lease descriptor once; no source changes or lock stealing occurs. */
void relocation_transaction_release(RelocationTransaction *);
#endif
