/** @file
 * @brief Separate delivery work scheduling, structural membership and test admission joins.
 * [PKG-08](../../../../../spec/project/packages.md#pkg-08) closes one finite set;
 * [ARCH-03](../../../../../spec/implementation/compiler.md#arch-03) keeps all
 * partial preparation under the enclosing operation.
 * Structural joins retain context, complete units and whole-version inputs.
 * [PKG-07](../../../../../spec/project/packages.md#pkg-07)
 */
#ifndef AMA_ACQUISITION_CLOSURE_PLAN_INTERNAL_H
#define AMA_ACQUISITION_CLOSURE_PLAN_INTERNAL_H
#include "plan.h"
/** Preserve the first failure explanation; no partial output is admitted under
 * [PKG-08](../../../../../spec/project/packages.md#pkg-08). */
AcquisitionClosureStatus acquisition_closure_fail(AcquisitionClosure *, AcquisitionClosureStatus, const char *);
/** Join complete structural/context/submodule evidence before following resolved
 * edges under [PKG-07](../../../../../spec/project/packages.md#pkg-07). */
AcquisitionClosureStatus acquisition_closure_structure(AcquisitionClosure *, const AcquisitionRoleRecord *, const AcquisitionExpansion *);
/** Require an own checked definition, or explicit noncontradictory draft proof.
 * NULL output denotes only a certified inactive main under
 * [PKG-07](../../../../../spec/project/packages.md#pkg-07).
 */
AcquisitionClosureStatus acquisition_closure_definition(AcquisitionClosure *, const AcquisitionRoleRecord *,
    const AcquisitionExpansion *, const DeliverySourceDefinition **);
/** Classify direct candidates and admit concrete tests, retain the complete associated directory and
 * schedule its active definitions without tests-of-tests under
 * [PKG-08](../../../../../spec/project/packages.md#pkg-08). */
AcquisitionClosureStatus acquisition_closure_tests(AcquisitionClosure *, const AcquisitionRoleRecord *, const AcquisitionExpansion *,
    const AcquisitionClosureProvider *);
/** Submit every captured entry with its exact immutable-source/request cause;
 * conflicts or failed allocation abort [PKG-07](../../../../../spec/project/packages.md#pkg-07) preparation. */
AcquisitionClosureStatus acquisition_closure_files(AcquisitionClosure *, const AcquisitionUnitEntry *,
    const AcquisitionRoleRecord *, const AcquisitionVersionSource *, AcquisitionMemberKind);
/** Activate provider-certified declared mains after retaining structural contexts,
 * independently of semantic-edge policy under
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02).
 */
AcquisitionClosureStatus acquisition_closure_declared_modules(AcquisitionClosure *,
    const AcquisitionRoleRecord *, const AcquisitionDeclaredModule *);
/** Add a canonical identity and preserve role-join allocation refusal under
 * [PKG-08](../../../../../spec/project/packages.md#pkg-08). */
AcquisitionClosureStatus acquisition_closure_request(AcquisitionClosure *, const char *, AcquisitionRole, AcquisitionReasonKind,
    const AcquisitionRoleRecord *, const AcquisitionRoleRecord **);
/** Expand one explicit whole version, including an empty source tree, without
 * inventing a module identity under [PKG-07](../../../../../spec/project/packages.md#pkg-07).
 */
AcquisitionClosureStatus acquisition_closure_version(AcquisitionClosure *, const AcquisitionRoleRecord *, const AcquisitionExpansion *);
#endif
