/** @file
 * @brief Own independently decoded relocation recovery state and its persistent result report.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * rolls back uncommitted changes and completes committed cleanup idempotently without rewriting source text again.
 */
#ifndef AMA_RELOCATION_RECOVERY_H
#define AMA_RELOCATION_RECOVERY_H
#include "../transaction/transaction.h"
/** Explicit recovery authority; journal names an existing private transaction directory, not a source namespace. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
typedef struct {
    const char *journal;
    RelocationIO io;
} RelocationRecoveryRequest;
/** Independently owned decoded state. Destruction releases memory/leases but never discards pending saved originals.
 * The transaction's report is owned and remains usable until recovery destruction.
 * Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
typedef struct {
    RelocationTransaction transaction;
    SourceManager sources;
    const SourceText *record;
    bool preparing;
    bool attempted;
    bool ready;
    bool finished;
    bool rolled_back;
} RelocationRecovery;
/** Initialize decoding/application ownership without filesystem effects. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
void relocation_recovery_init(RelocationRecovery *, SourceAllocator);
/** Release independent metadata/source owners and outstanding leases, retaining incomplete disk state. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
void relocation_recovery_destroy(RelocationRecovery *);
/** Recover one owned journal under exclusive access; false retains diagnostics and all unresolved saved data. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_recovery_run(RelocationRecovery *, const RelocationRecoveryRequest *);
#endif
