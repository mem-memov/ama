/** @file
 * @brief Check explicit client entry/export dispositions before original source removal.
 * [TOOL-03](../../../../spec/implementation/tools-and-conformance.md#tool-03)
 * treats client selections as additional roots: dropping or mapping them must be
 * explicit. Final destination existence remains the final graph check's obligation.
 */
#ifndef AMA_PROJECT_SOURCE_REMOVAL_ROOTS_H
#define AMA_PROJECT_SOURCE_REMOVAL_ROOTS_H
#include "check.h"
/** Preserve entry/export provenance even when multiple client selections name one module. */
typedef enum { SOURCE_CLIENT_ENTRY, SOURCE_CLIENT_EXPORT } SourceClientRootKind;
/** Only explicit client-authorized removal or mapping may release an original root. */
typedef enum { SOURCE_CLIENT_KEEP, SOURCE_CLIENT_DROP, SOURCE_CLIENT_MAP } SourceClientRootAction;
/** One finite prepared client selection. definition must be an exact member of
 * the complete original checked export. destination is canonical and non-NULL
 * exactly for MAP; the enclosing operation must validate its final availability.
 * All data is borrowed through report consumption. Mapping never removes an
 * authored original dependency blocker under
 * [TOOL-03](../../../../spec/implementation/tools-and-conformance.md#tool-03).
 */
typedef struct SourceClientRoot {
    struct SourceClientRoot *next;
    const DeliverySourceDefinition *definition;
    SourceClientRootKind kind;
    SourceClientRootAction action;
    const char *destination;
} SourceClientRoot;
/** One unchanged original selection that would become dangling; node is arena-owned. */
typedef struct SourceClientRootBlocker {
    struct SourceClientRootBlocker *next;
    const SourceClientRoot *root;
} SourceClientRootBlocker;
/** Check every client selection separately against the exact original removal
 * set. KEEP blocks exactly when its original definition is removed. DROP/MAP
 * reflect already explicit client decisions, never an inferred replacement.
 * Validate original handle membership, categories, destinations and removal keys.
 * Output changes only on OK/BLOCKED, in client order; failure scratch stays owned.
 * Reject cyclic request lists before traversal. No final graph availability or
 * source-topology claim is made. For N original
 * definitions, Q roots and R removed keys, time is O(Q*(N+R)) comparisons plus
 * canonical spelling validation, storage O(blocking occurrences). No I/O occurs.
 */
SourceRemovalStatus source_removal_client_roots(SourceArena *, const DeliverySourceDefinition *,
    const SourceClientRoot *, const char *const *removed, size_t count, const SourceClientRootBlocker **output);
#endif
