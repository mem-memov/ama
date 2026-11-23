/** @file
 * @brief Retain explicit entry/export dispositions against actual final compiler facts.
 * [TOOL-01](../../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * reports client selections without patching unknown configuration formats;
 * [TOOL-03](../../../../../spec/implementation/tools-and-conformance.md#tool-03)
 * requires explicit drop/map decisions independently of authored consumers.
 */
#ifndef AMA_SOURCE_OPERATION_CLIENTS_H
#define AMA_SOURCE_OPERATION_CLIENTS_H
#include "../plan/plan.h"
/** One reported client choice; NULL final means explicitly dropped, never missing.
 * Preserve repeated selections separately, including entry and export of one Type. */
typedef struct SourceOperationClientSetting {
    struct SourceOperationClientSetting *next;
    const SourceClientRoot *request;
    const DeliverySourceDefinition *final;
} SourceOperationClientSetting;
/** Distinguish malformed facts, original cut blockers and missing final selections. */
typedef enum { SOURCE_OPERATION_CLIENTS_OK, SOURCE_OPERATION_CLIENTS_INVALID,
    SOURCE_OPERATION_CLIENTS_INPUTS, SOURCE_OPERATION_CLIENTS_BLOCKED,
    SOURCE_OPERATION_CLIENTS_MISSING, SOURCE_OPERATION_CLIENTS_MEMORY } SourceOperationClientsStatus;
/** Own finite report nodes, borrowing immutable plan/client/request owners through
 * destruction. valid proves named definition availability, not executable-entry
 * specialization, ABI compatibility, writable external configuration or publication. */
typedef struct {
    SourceArena arena;
    const SourceOperationPlan *plan;
    const SourceCompilerClient *compiler;
    SourceOperationClientSetting *first, *last;
    const SourceClientRoot *failed;
    const SourceClientRootBlocker *blockers;
    SourceOperationClientsStatus status;
    bool attempted, valid;
} SourceOperationClients;
/** Begin an empty effect-free report owner. */
void source_operation_clients_init(SourceOperationClients *, SourceAllocator);
/** Release nodes before their borrowed evidence expires; safe after failure. */
void source_operation_clients_destroy(SourceOperationClients *);
/** Build once from a prepared image and independent exact final compiler reply.
 * Recheck original client ownership and D blockers using the common removal law.
 * KEEP requires its original identity in final; MAP requires its explicit target;
 * DROP records no target. Missing selections refuse even when final code compiles.
 * Refuse cyclic request lists before traversing them. No I/O, executions, edits or
 * source mutations. For Q selections, N originals, F final definitions and D
 * removed names, time O(Q*(N+F+D)) plus exact response/capture matching, storage
 * O(Q) report/blocker nodes. All scans terminate on finite admitted inputs.
 */
SourceOperationClientsStatus source_operation_clients_build(SourceOperationClients *,
    const SourceOperationPlan *, const SourceCompilerClient *);
#endif
