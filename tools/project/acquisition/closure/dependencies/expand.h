/** @file
 * @brief Expand source-delivery membership using actual resolved compiler edges.
 * [PKG-08](../../../../../spec/project/packages.md#pkg-08) uses the compiler's
 * [DEP-03](../../../../../spec/implementation/compiler.md#dep-03) relation rather
 * than textual reference scans or guessed execution reachability.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * lets explicit source-copy modes retain unchanged external targets.
 */
#ifndef AMA_ACQUISITION_DEPENDENCY_EXPANSION_H
#define AMA_ACQUISITION_DEPENDENCY_EXPANSION_H
#include "../roles/roles.h"
#include "delivery/source/records.h"
/** Pure explicit operation policy over already checked edges. True requests an
 * included unit; false preserves the original edge as external to this delivery
 * set. This never changes bindings or authorizes ignoring compiler errors under
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02).
 */
typedef bool (*AcquisitionDependencySelection)(void *, const DeliverySourceDefinition *, const DeliverySourceDependency *);
/** Add each direct resolved dependency of one checked module as an active unit,
 * recording its original requirement and requesting delivery record. NULL selection
 * follows all edges for normal acquisition; an explicit source-operation policy
 * can retain unselected dependencies externally without inventing their tests. Require a
 * checked plain export and a matching active role handle.
 * Original source owners must outlive the role evidence. No edge, path, scope or
 * selector is reinterpreted here; the compiler has already applied PKG-09 and the
 * original defining configuration. All edge kinds, including static producers
 * and constant namespaces, participate. Duplicate causes are idempotent; queue
 * identity handles source cycles. Failure may leave earlier successful edges,
 * so the enclosing operation must abort, not publish a partial delivery closure.
 * Cost is one role insertion per direct compiler edge; no files/network/commands
 * are touched. [PKG-08](../../../../../spec/project/packages.md#pkg-08) assigns
 * transitive expansion to the finite active queue, not recursive calls here.
 */
AcquisitionRoleStatus acquisition_dependencies_expand(AcquisitionRoles *,
    const AcquisitionRoleRecord *, const DeliverySourceDefinition *, AcquisitionDependencySelection, void *context);
#endif
