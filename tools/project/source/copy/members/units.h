/** @file
 * @brief Join actual mapped structural-unit causes into one coherent physical file relation.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * requires overlapping maps to agree; [PKG-07](../../../../../spec/project/packages.md#pkg-07)
 * supplies complete raw unit members without parsing opaque contents.
 */
#ifndef AMA_SOURCE_COPY_UNIT_MEMBERS_H
#define AMA_SOURCE_COPY_UNIT_MEMBERS_H
#include "../closure.h"
#include "../../mapping/map.h"
#include "../../mapping/files/files.h"
#include "../../mapping/unit/member.h"
#include "../contexts/omissions.h"
/** Keep missing main correspondence and unit/file-map failures distinct. */
typedef enum { SOURCE_COPY_UNITS_OK, SOURCE_COPY_UNITS_INVALID, SOURCE_COPY_UNITS_MAIN,
    SOURCE_COPY_UNITS_PATH, SOURCE_COPY_UNITS_MEMORY, SOURCE_COPY_UNITS_MAP, SOURCE_COPY_UNITS_CONTEXT } SourceCopyUnitsStatus;
/** Borrow exact original cause/clone witnesses and map-arena-owned proposed path. */
typedef struct {
    const AcquisitionMember *member;
    const AcquisitionMemberCause *cause;
    const SourceCloneMapping *clone;
    const char *path;
    SourceUnitPathStatus path_status;
    SourceCopyContextStatus context_status;
} SourceCopyUnitsIssue;
/** Join every UNIT_MEMBER cause of a valid original closure with its explicitly
 * mapped original main. Clone map must be ready over that same original capture;
 * physical file map stays open for later test/declared/environment passes. Missing
 * active main rows refuse. An unmapped context may be omitted only when the
 * caller supplies a matching open omission owner; record that obligation before
 * skipping its material. Final discovery/context proof remains mandatory before
 * acceptance. NULL omission owner retains strict missing-context refusal. No other cause family is silently mapped. Each overlap uses the
 * same original kind/bytes and must propose the identical output. Failure poisons
 * file-map sealing and retains exact witnesses/prefix. Invalid closed/failed map
 * or missing issue refuses unchanged. Inputs outlive result inspection/teardown.
 * No I/O, discovery, text parsing, name guessing or publication. Finite cause/main/
 * file scans plus checked owned key joins bound work and storage.
 */
SourceCopyUnitsStatus source_copy_map_unit_members(SourceFileMap *, const SourceCopyClosure *,
    const SourceCloneMap *, SourceCopyContextOmissions *, SourceCopyUnitsIssue *);
#endif
