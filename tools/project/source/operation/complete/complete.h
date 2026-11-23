/** @file
 * @brief Own the single complete read-only source-operation certificate consumed by preview/publication.
 * [TOOL-04](../../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * joins explicit intent, authorized writes, original safety, settings and actual
 * final compiler proofs before mutation. Freshness, leases, staging, publication
 * and recovery remain distinct subsequent transaction requirements.
 */
#ifndef AMA_SOURCE_OPERATION_COMPLETE_H
#define AMA_SOURCE_OPERATION_COMPLETE_H
#include "../writes/join/join.h"
#include "../material/material.h"
#include "../rebuild/rebuild.h"
/** Explicit immutable request and genuine prerequisite owners. The write inputs
 * contain no untyped extra mutations. final_roots is an exact explicit F setting;
 * configured is O; writes.observed->original is the single analyzed U catalog.
 * Borrow all owners/arrays through complete-owner destruction. No supported
 * writable external root/client configuration format is invented by this API. */
typedef struct {
    const SourceOperationWritesInputs *writes;
    const SourceOperationIntent *intent;
    const SourceAnalysis *original;
    const SourceRootCapture *configured;
    const ModuleRoot *final_roots;
    size_t final_root_count;
    const SourceCopyContextOmissions *contexts;
    const SourceRemovalInputUse *inputs;
    const SourceClientRoot *clients;
    const SourceOperationRootDisposition *root_originals;
    size_t root_original_count;
    const SourceOperationRootAddition *root_additions;
    size_t root_addition_count;
} SourceOperationCompleteInputs;
/** Distinguish where an expectation's authority originates, not just set equality. */
typedef enum { SOURCE_OPERATION_SCOPE_WRITER, SOURCE_OPERATION_SCOPE_PRESERVED,
    SOURCE_OPERATION_SCOPE_INITIALIZED, SOURCE_OPERATION_SCOPE_SUPPLIER } SourceOperationScopeOrigin;
/** Retain a typed witness for every actual final scope. baseline is the genuine
 * original preserved scope, or the first selected scope of an unchanged supplier.
 * SUPPLIER remains pending the mandatory final untouched-supplier proof; it is
 * never new source authority. */
typedef struct {
    SourceOperationScopeOrigin kind;
    const SourceOperationConfigurationWrite *writer;
    const SourcePackageScope *baseline;
    const SourceVersionInitialization *initialization;
} SourceOperationScopeAuthority;
/** Preserve each failed composed gate with its child owner's detailed evidence. */
typedef enum { SOURCE_OPERATION_COMPLETE_OK, SOURCE_OPERATION_COMPLETE_INVALID,
    SOURCE_OPERATION_COMPLETE_WRITES, SOURCE_OPERATION_COMPLETE_ENVIRONMENT,
    SOURCE_OPERATION_COMPLETE_PLAN, SOURCE_OPERATION_COMPLETE_CONFIGURATION,
    SOURCE_OPERATION_COMPLETE_MATERIAL, SOURCE_OPERATION_COMPLETE_ROOTS,
    SOURCE_OPERATION_COMPLETE_PROOF, SOURCE_OPERATION_COMPLETE_CLIENTS,
    SOURCE_OPERATION_COMPLETE_REBUILD, SOURCE_OPERATION_COMPLETE_MEMORY } SourceOperationCompleteStatus;
/** One noncopyable owner; destroy children in reverse borrowing order. prepared
 * means all read-only construction gates passed; only sealed certifies the exact
 * independent compiler/site proof, final client choices and rebuild intentions.
 * Neither flag certifies live freshness or recoverable publication. After sealed,
 * all inputs/results remain immutable; retries do not mutate accepted evidence. */
typedef struct {
    SourceArena arena;
    SourceOperationCompleteInputs inputs;
    SourceOperationWrites writes;
    SourcePackageEnvironment original_environment;
    SourceOperationPlan plan;
    SourceOperationMaterialReview material;
    SourceOperationRoots roots;
    SourceOperationClients clients;
    SourceOperationRebuild rebuild;
    SourceOperationConfigurationExpectation *configurations;
    SourceOperationScopeAuthority *authorities;
    size_t configuration_count;
    const char *failed_path;
    const SourceCompilerClient *compiler;
    SourceCopyProofStatus proof_status;
    SourceCopyProofIssue proof_issue;
    SourceSelectionRootsStatus selection_status;
    SourceSelectionRootsIssue selection_issue;
    SourceOperationCompleteStatus status;
    bool attempted, prepared, seal_attempted, sealed;
} SourceOperationComplete;
/** Initialize every child without discovery, execution or source effects. */
void source_operation_complete_init(SourceOperationComplete *, SourceAllocator);
/** Retire reports/proofs before plan image, edits and borrowed producers expire. */
void source_operation_complete_destroy(SourceOperationComplete *);
/** Prepare once: typed write join, original environment, one simultaneous
 * projection, complete configuration authority/set derivation, mutation forest,
 * exact request/material reconstruction and explicit O/U/F settings. Require a
 * typed recipe for carried package configs and active copied module INI files.
 * No module/test/native/CMake/network execution or source mutation occurs.
 * No caller-invented expected permission sets or second projection is accepted.
 * Finite child bounds plus pairwise scope/writer/map scans; owned storage is the
 * sum of child bounds and O(final scopes) authority/expectation rows.
 */
SourceOperationCompleteStatus source_operation_complete_prepare(SourceOperationComplete *, const SourceOperationCompleteInputs *);
/** Seal once after preparation using an independently successful compiler client
 * for exactly plan.capture.view. Run all shared actual material/context/site/J
 * proofs, resolve explicit client choices and retain domain-wide owned rebuild
 * intentions. No compiler invocation or execution occurs here. Child proof bounds
 * determine finite work/storage. Failure stays unsealed with destructible precise
 * evidence; a different reply requires a fresh complete owner. Successful sealing
 * is the immutable handoff to preview/transaction, never permission to skip live
 * freshness, exclusive access or recovery checks.
 */
SourceOperationCompleteStatus source_operation_complete_seal(SourceOperationComplete *, const SourceCompilerClient *);
#endif
