/** @file
 * @brief Own monotone delivery roles and their one-time expansion queue.
 * [PKG-07](../../../../../spec/project/packages.md#pkg-07) distinguishes retained
 * topology from active units; [PKG-08](../../../../../spec/project/packages.md#pkg-08)
 * requires each active identity to expand once, including promoted ancestors.
 * Distinct causal edges preserve operation evidence.
 * [PROJECT-03](../../../../../spec/implementation/project-tools.md#project-03)
 * One operation owns complete and failed metadata.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03)
 */
#ifndef AMA_ACQUISITION_ROLES_H
#define AMA_ACQUISITION_ROLES_H
#include "source/memory/arena.h"
#include "source/text/source.h"

/** A context preserves topology; an active record owns dependency/test expansion.
 * These are the two retained states of [PKG-08](../../../../../spec/project/packages.md#pkg-08). */
typedef enum { ACQUISITION_CONTEXT, ACQUISITION_ACTIVE } AcquisitionRole;
/** Explicit edge labels explain why a unit belongs to the least closure under
 * [PKG-07](../../../../../spec/project/packages.md#pkg-07) and
 * [PKG-08](../../../../../spec/project/packages.md#pkg-08). */
typedef enum { ACQUISITION_SELECTED, ACQUISITION_TOPOLOGY, ACQUISITION_SUBMODULE,
    ACQUISITION_DEPENDENCY, ACQUISITION_TEST } AcquisitionReasonKind;
/** Stable role identity lets causal edges refer to a record before its complete
 * layout. [PKG-08](../../../../../spec/project/packages.md#pkg-08) retains shared
 * identity through source cycles without copying or re-expanding the record.
 */
typedef struct AcquisitionRoleRecord AcquisitionRoleRecord;
/** An operation-owned causal edge. A NULL requester denotes an external selection;
 * the source owner of origin must outlive this registry. Equal edges are retained
 * once, preserving all distinct causes required by
 * [PROJECT-03](../../../../../spec/implementation/project-tools.md#project-03). */
typedef struct AcquisitionReason {
    struct AcquisitionReason *next;
    const AcquisitionRoleRecord *requester;
    AcquisitionReasonKind kind;
    SourceSpan origin;
} AcquisitionReason;
/** Stable owned identity, monotone role and immutable causal edges. Clients borrow
 * records and must not mutate links/state. Expansion does not remove records,
 * allowing cycles to refer to an already expanded identity under
 * [PKG-08](../../../../../spec/project/packages.md#pkg-08). */
struct AcquisitionRoleRecord {
    AcquisitionRoleRecord *next, *pending_next;
    const char *identity;
    AcquisitionRole role;
    bool expanded;
    AcquisitionReason *reasons;
};
/** Single-executor owner; source spans borrow their original immutable snapshots.
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) keeps failed
 * scratch operation-owned until destruction. */
typedef struct {
    SourceArena arena;
    AcquisitionRoleRecord *records, *pending, *last_pending;
} AcquisitionRoles;
/** Failed insertion publishes no record, edge, promotion or queue mutation. */
typedef enum { ACQUISITION_ROLE_OK, ACQUISITION_ROLE_INVALID,
    ACQUISITION_ROLE_OUT_OF_MEMORY } AcquisitionRoleStatus;
/** Begin an empty registry; allocator callbacks must outlive it. */
void acquisition_roles_init(AcquisitionRoles *, SourceAllocator);
/** Release all records/edges and expire every borrowed registry handle. */
void acquisition_roles_destroy(AcquisitionRoles *);
/** Add one cause for an already normalized complete module/version selection identity. The caller
 * supplies canonical identities from the common resolver, never physical paths. Version
 * identities are used only by explicit whole-version selection.
 * requester is NULL or a record in this owner; origin is absent or a valid span.
 * Identity bytes are copied. Success joins the previous role with the requested
 * role; equal causal edges are idempotent. Output changes only on success.
 * Linear registry/edge lookup, O(identity bytes) new storage per new identity and
 * O(1) per new cause; arithmetic/allocation failure preserves visible state.
 * [PKG-08](../../../../../spec/project/packages.md#pkg-08) makes this join monotone.
 */
AcquisitionRoleStatus acquisition_roles_add(AcquisitionRoles *, const char *identity,
    AcquisitionRole, AcquisitionReasonKind, const AcquisitionRoleRecord *requester,
    SourceSpan origin, const AcquisitionRoleRecord **output);
/** Claim the next active record once, or NULL at the current fixed point.
 * Consumers must abort the enclosing operation if expansion fails; this function
 * is not a retry/rollback mechanism. Newly discovered work may be added while
 * expanding. Context-only records are never returned, implementing the finite
 * progress invariant of [PKG-08](../../../../../spec/project/packages.md#pkg-08).
 */
const AcquisitionRoleRecord *acquisition_roles_take(AcquisitionRoles *);
#endif
