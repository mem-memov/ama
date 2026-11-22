/** @file
 * @brief Retain unique delivery inputs with every structural and source-snapshot cause.
 * [PKG-07](../../../../../spec/project/packages.md#pkg-07) defines unit bytes;
 * [PROJECT-03](../../../../../spec/implementation/project-tools.md#project-03)
 * requires recorded memberships and immutable source provenance.
 * Bound source contracts preserve coherent version provenance.
 * [PKG-06](../../../../../spec/project/packages.md#pkg-06)
 * Set union merges overlapping inputs without losing causes.
 * [PKG-08](../../../../../spec/project/packages.md#pkg-08)
 * This owner performs no compiler, transport or source mutation.
 * [PROJECT-01](../../../../../spec/implementation/project-tools.md#project-01)
 */
#ifndef AMA_ACQUISITION_MEMBERS_H
#define AMA_ACQUISITION_MEMBERS_H
#include "../unit/unit.h"
#include "../../sources/sources.h"
/** Reasons describe actual capture operations, not inferred neighboring assets
 * under [PKG-07](../../../../../spec/project/packages.md#pkg-07). */
typedef enum { ACQUISITION_UNIT_MEMBER, ACQUISITION_TEST_MEMBER,
    ACQUISITION_VERSION_CONFIGURATION, ACQUISITION_VERSION_GUIDANCE, ACQUISITION_DECLARED_INPUT } AcquisitionMemberKind;
/** Borrow the requesting active/context role and immutable source contract through
 * operation teardown. NULL source denotes an explicitly supplied standard root or separately
 * identified unversioned external input, never an unresolved ordinary version. [PKG-06](../../../../../spec/project/packages.md#pkg-06)
 * fixes ordinary version origins before membership publication.
 */
typedef struct AcquisitionMemberCause {
    struct AcquisitionMemberCause *next;
    const AcquisitionRoleRecord *requester;
    const AcquisitionVersionSource *source;
    AcquisitionMemberKind kind;
} AcquisitionMemberCause;
/** Unique byte-sorted physical input with captured bytes and all inclusion causes.
 * Path belongs to this arena; content borrows the capture's SourceManager. Directory
 * content is NULL and file content is present even for empty files, preserving
 * [PKG-07](../../../../../spec/project/packages.md#pkg-07) faithfully.
 */
typedef struct AcquisitionMember {
    struct AcquisitionMember *next;
    const char *path;
    const SourceText *content;
    bool directory;
    AcquisitionMemberCause *causes;
} AcquisitionMember;
/** Single-executor owner; no disk/network mutation or compiler callbacks belong
 * in this set under [PROJECT-01](../../../../../spec/implementation/project-tools.md#project-01).
 */
typedef struct { SourceArena arena; AcquisitionMember *first; } AcquisitionMembers;
/** Conflict refuses changed bytes/kind at an already retained exact path. */
typedef enum { ACQUISITION_MEMBER_OK, ACQUISITION_MEMBER_INVALID,
    ACQUISITION_MEMBER_CONFLICT, ACQUISITION_MEMBER_OUT_OF_MEMORY } AcquisitionMemberStatus;
/** Initialize one empty member set with explicit allocator authority. */
void acquisition_members_init(AcquisitionMembers *, SourceAllocator);
/** Release complete and failed metadata before borrowed captures/roles/sources. */
void acquisition_members_destroy(AcquisitionMembers *);
/** Join one captured PRESENT structural entry and its cause. requester must be
 * non-NULL and belongs to the enclosing operation; source is that operation's
 * bound ordinary version or NULL for an explicit standard root/unversioned input. Borrowed
 * owners outlive this set; only paths and cause records are copied here.
 * Equal paths require equal kind and exact file bytes. Equal causes are idempotent;
 * distinct causes remain visible. Prepare every allocation before changing the
 * public set. Failure preserves all prior memberships/causes and output. Linear
 * sorted lookup plus byte equality; O(path + one record/cause) new owned storage.
 * [PKG-08](../../../../../spec/project/packages.md#pkg-08) requires a least set,
 * not duplicate physical files for overlapping structural units.
 */
AcquisitionMemberStatus acquisition_members_add(AcquisitionMembers *, const AcquisitionUnitEntry *,
    const AcquisitionRoleRecord *, const AcquisitionVersionSource *, AcquisitionMemberKind,
    const AcquisitionMember **output);
#endif
