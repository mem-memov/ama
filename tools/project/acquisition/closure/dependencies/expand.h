/** @file
 * @brief Expand source-delivery membership using actual resolved compiler edges.
 * [PKG-08](../../../../../spec/project/packages.md#pkg-08) uses the compiler's
 * [DEP-03](../../../../../spec/implementation/compiler.md#dep-03) relation rather
 * than textual reference scans or guessed execution reachability.
 */
#ifndef AMA_ACQUISITION_DEPENDENCY_EXPANSION_H
#define AMA_ACQUISITION_DEPENDENCY_EXPANSION_H
#include "../roles/roles.h"
#include "delivery/source/records.h"
/** Add each direct resolved dependency of one checked module as an active unit,
 * recording its original requirement and requesting delivery record. Require a
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
    const AcquisitionRoleRecord *, const DeliverySourceDefinition *);
#endif
