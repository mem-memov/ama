/** @file
 * @brief Check that physical main proposals and nominal clone proposals describe the same originals.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * requires one simultaneous clone map for every copied definition, including
 * modules retained as declared data. Opaque companions do not create definitions.
 */
#ifndef AMA_PROJECT_SOURCE_MAPPING_CORRESPONDENCE_H
#define AMA_PROJECT_SOURCE_MAPPING_CORRESPONDENCE_H
#include "map.h"
#include "files/files.h"
/** Distinguish incomplete relations, contradictory observations and invalid owners. */
typedef enum { SOURCE_CORRESPONDENCE_OK, SOURCE_CORRESPONDENCE_INVALID,
    SOURCE_CORRESPONDENCE_MISSING, SOURCE_CORRESPONDENCE_CHANGED } SourceCorrespondenceStatus;
/** Borrow the exact original main and whichever proposals witness a refusal. */
typedef struct {
    const SourceCapturedUnit *unit;
    const SourceFileMapping *file;
    const SourceCloneMapping *clone;
} SourceCorrespondenceIssue;
/** Require both sealed relations. For every originally indexed main, membership
 * in the physical input map is equivalent to membership in the clone map, final
 * physical keys agree, and the original regular-file observation equals the
 * captured main bytes. This includes drafts and independently retained module
 * data; ordinary opaque inputs need no nominal row. Reset issue on entry and
 * retain the first exact refusal. No allocations, I/O, source edits or final
 * catalog authority. Time O(U*(F+C)) path comparisons plus observed main bytes,
 * storage O(1), for U originals, F files and C clones.
 */
SourceCorrespondenceStatus source_mapping_correspondence(const SourceCloneMap *,
    const SourceFileMap *, SourceCorrespondenceIssue *);
#endif
