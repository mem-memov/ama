/** @file
 * @brief Join explicit physical tree placement to nominal clone proposals.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * maps each original definition through its actual physical main, not a renamed
 * logical prefix. Final catalog and transaction gates remain separate.
 */
#ifndef AMA_PROJECT_SOURCE_MAPPING_PROPOSE_H
#define AMA_PROJECT_SOURCE_MAPPING_PROPOSE_H
#include "map.h"
#include "tree.h"
#include "identity.h"
/** Borrowed first failing original/physical placement, with structural decoding
 * status when that stage refused. No witness grants final catalog authority.
 */
typedef struct {
    const SourceCapturedUnit *unit;
    const char *path;
    SourceIdentityStatus identity_status;
} SourceCloneTreeIssue;
/** Add every explicitly selected original main to an unsealed relation over the
 * exact same original capture. Match raw entry paths exactly, decode each final
 * physical main under explicit roots, and admit it through nominal/physical
 * injectivity checks. Equal proposals are allowed; no seal is performed because
 * context/test/dependency placements may still follow. Scratch identities belong
 * to the map arena. Refusal poisons the relation and retains its destructible
 * prefix and borrowed issue; no source reads, edits or destination writes occur.
 * Success proves candidate correspondence only: final common discovery must
 * confirm selection, topology, aliases and every resulting logical identity.
 * For U originals, E entries, R roots and M existing rows, time is bounded by
 * O(U*(E+R+M)) path comparisons plus copied path bytes; storage is O(U*path bytes).
 */
SourceCloneStatus source_clone_map_propose_tree(SourceCloneMap *, const SourceTreePlacement *,
    const ModuleRoot *, size_t, SourceCloneTreeIssue *);
#endif
