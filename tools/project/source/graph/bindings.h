/** @file
 * @brief Check simultaneous original/copy binding laws on complete admitted compiler exports.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * preserves original edges and maps a copied edge's target exactly when the
 * target is also copied. Site/edit, byte, package and transaction proofs remain
 * separate [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * obligations; this predicate certifies only the ordered binding projection.
 */
#ifndef AMA_PROJECT_SOURCE_GRAPH_BINDINGS_H
#define AMA_PROJECT_SOURCE_GRAPH_BINDINGS_H
#include "../mapping/map.h"
#include "delivery/source/records.h"
/** Exact semantic mismatch stage, independent of diagnostics text. */
typedef enum { SOURCE_BINDINGS_OK, SOURCE_BINDINGS_INVALID, SOURCE_BINDINGS_MISSING,
    SOURCE_BINDINGS_EXTRA, SOURCE_BINDINGS_COLLISION, SOURCE_BINDINGS_KIND,
    SOURCE_BINDINGS_EDGE, SOURCE_BINDINGS_REMOVED_TARGET } SourceBindingsStatus;
/** Borrow first original/final/clone/edge witnesses and expected complete identity. */
typedef struct {
    const DeliverySourceDefinition *original, *final;
    const SourceCloneMapping *clone;
    const DeliverySourceDependency *original_edge, *final_edge;
    const char *expected;
} SourceBindingsIssue;
/** Check complete unique checked original/final exports and a sealed clone map
 * over that original capture. The normalized removed-name array may also contain
 * drafts absent from active exports. Each surviving active original and active
 * clone must appear exactly once in final; other final definitions refuse.
 * Survivor/copy identity overlap refuses even when bytes or origins match.
 * Definition categories, ordered edge multiplicity, implicit flags and normalized
 * constant members are preserved. A survivor keeps original targets; a copy uses
 * C(target) iff target belongs to the clone relation.
 * A required original target must survive; a replacement with the same name is
 * not that original definition. Identity-preserving mapped targets remain valid.
 * No recursive edge traversal, allocation or I/O. Clear issue on entry and
 * publish the first exact mismatch.
 * This does not prove edge source-span translation or unchanged original bytes;
 * callers must separately validate parsed edits and final compiler input capture.
 * With N originals, F final definitions, C clones, E checked edge occurrences
 * and R removals, time is O((N+C)*(N+F)+(N+F)*R+E*C) key comparisons;
 * scratch is O(1). All names, spans and owners outlive issue consumption.
 */
SourceBindingsStatus source_graph_bindings(const DeliverySourceDefinition *original,
    const DeliverySourceDefinition *final, const SourceCloneMap *,
    const char *const *removed, size_t removed_count, SourceBindingsIssue *);
#endif
