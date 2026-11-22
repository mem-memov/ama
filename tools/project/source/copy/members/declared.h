/** @file
 * @brief Join explicitly declared raw inputs with structural and version-local copy placement.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * gives structural unit/test maps priority and preserves other version-relative
 * paths. [PKG-07](../../../../../spec/project/packages.md#pkg-07) forbids inventing
 * undeclared asset ownership or parsing native code to infer it.
 */
#ifndef AMA_SOURCE_COPY_DECLARED_MEMBERS_H
#define AMA_SOURCE_COPY_DECLARED_MEMBERS_H
#include "../destinations/destinations.h"
#include "../../mapping/files/files.h"
#include "../../mapping/physical/version.h"
/** Distinguish a required explicit placement from malformed original authority. */
typedef enum { SOURCE_COPY_DECLARED_OK, SOURCE_COPY_DECLARED_INVALID,
    SOURCE_COPY_DECLARED_UNPLACED, SOURCE_COPY_DECLARED_VERSION,
    SOURCE_COPY_DECLARED_PATH, SOURCE_COPY_DECLARED_MEMORY,
    SOURCE_COPY_DECLARED_MAP } SourceCopyDeclaredStatus;
/** Borrow original cause/candidate; proposed path belongs to the output arena. */
typedef struct {
    const AcquisitionMember *member;
    const AcquisitionMemberCause *cause;
    const SourceCopyDestination *requester;
    const char *path;
} SourceCopyDeclaredIssue;
/** Join declared non-configuration members once after all explicit/unit/test maps
 * have been added. Existing placements are authoritative structural/explicit
 * input maps; compare declared kind/bytes through ordinary insertion. For a
 * member without such a placement, derive a version-relative destination for
 * every declared cause from its original bound version and the requester's
 * designated final version. All proposed destinations for a shared input must
 * agree. An unversioned input outside existing maps returns UNPLACED: the outer
 * planner must supply its explicit placement rather than invent a relative base.
 * This preparation outcome defines no new command flag or default input policy.
 *
 * Package configurations are environments and are left to the separate exact
 * configuration pass, even if mentioned as declared inputs. Other cause kinds
 * remain untouched. The valid destination result, original closure and version
 * table remain immutable through map teardown; no I/O or source mutation occurs.
 * Output stays open for environment maps. Invalid closed/failed maps or missing
 * issue refuse unchanged; other failures preserve witnesses and poison sealing.
 * Finite member/cause/candidate/version/file scans bound time; owned translated
 * keys and adapters bound storage by the visited original/output path bytes.
 */
SourceCopyDeclaredStatus source_copy_map_declared_members(SourceFileMap *, const SourceCopyDestinations *,
    const SourceVersionPlacement *, size_t, SourceCopyDeclaredIssue *);
#endif
