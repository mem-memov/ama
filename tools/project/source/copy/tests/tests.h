/** @file
 * @brief Map complete associated test material through copied source ownership.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * chooses the deepest original association, while
 * [PKG-08](../../../../../spec/project/packages.md#pkg-08) mirrors logical source
 * components as ordinary physical test directories, without implicit plus signs.
 */
#ifndef AMA_SOURCE_COPY_TEST_MATERIAL_H
#define AMA_SOURCE_COPY_TEST_MATERIAL_H
#include "../closure.h"
#include "../../mapping/map.h"
#include "../../mapping/files/files.h"
#include "../../mapping/physical/main.h"
/** Preserve the failing authority rather than guessing a missing test location. */
typedef enum { SOURCE_COPY_TESTS_OK, SOURCE_COPY_TESTS_INVALID, SOURCE_COPY_TESTS_MAIN,
    SOURCE_COPY_TESTS_VERSION, SOURCE_COPY_TESTS_PATH, SOURCE_COPY_TESTS_MEMORY,
    SOURCE_COPY_TESTS_MAP } SourceCopyTestsStatus;
/** Witnesses borrow original owners; proposed path belongs to the file-map arena. */
typedef struct {
    const AcquisitionMember *member;
    const AcquisitionMemberCause *cause;
    const SourceCloneMapping *clone;
    const char *path;
} SourceCopyTestsIssue;
/** Join every captured TEST_MEMBER, including opaque bytes/empty directories,
 * into an open physical file relation. Require a valid copy closure, a ready main
 * relation over its exact capture, and unique canonical ordinary destination
 * versions with normalized absolute physical roots. All source requesters of test
 * material need a main correspondence and a matching destination version.
 * Source and test owners outlive inspection and file-map teardown.
 *
 * Build the full association set first, then choose the deepest containing
 * original physical directory independently of membership/cause order. Preserve
 * every remaining physical suffix byte. Shared unit/test mappings must agree via
 * normal file-map insertion; topology/discovery and final graph proof remain outer
 * gates. No companion guessing, automatic test '+' or dropped material.
 *
 * Result stays open for other material passes. Failure retains owned scratch and
 * witnesses and poisons sealing; a closed/failed map or missing issue refuses
 * unchanged. Success clears issue. No I/O, source mutation or executable tests.
 * Finite scans of members/causes/mains/versions/associations bound time; owned
 * association and output strings bound storage by the visited input/output bytes.
 */
SourceCopyTestsStatus source_copy_map_test_members(SourceFileMap *, const SourceCopyClosure *,
    const SourceCloneMap *, const SourceVersionPlacement *, size_t, SourceCopyTestsIssue *);
#endif
