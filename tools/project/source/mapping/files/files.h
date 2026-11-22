/** @file
 * @brief Own injective physical input/output proposals, including opaque material.
 * [TOOL-02](../../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * requires overlapping maps to agree and forbids different inputs from sharing
 * one output. Equal bytes never authorize merging distinct physical inputs.
 */
#ifndef AMA_PROJECT_SOURCE_MAPPING_FILES_H
#define AMA_PROJECT_SOURCE_MAPPING_FILES_H
#include "../tree.h"
/** One owned pair of prepared absolute keys; capture/content ownership is borrowed. */
typedef struct SourceFileMapping {
    struct SourceFileMapping *next;
    const AcquisitionUnitEntry *original;
    const char *source, *destination;
} SourceFileMapping;
/** Conflicts and failed preparation cannot become a sealed complete table. */
typedef enum { SOURCE_FILE_MAP_OK, SOURCE_FILE_MAP_INVALID, SOURCE_FILE_MAP_CONFLICT,
    SOURCE_FILE_MAP_MEMORY } SourceFileMapStatus;
/** Own strings/rows and retain first conflict witnesses through destruction.
 * ready proves injectivity and no final file-as-ancestor conflict only; it does
 * not prove replacement, final package authority, graph laws or publication safety.
 */
typedef struct {
    SourceArena arena;
    SourceFileMapping *first;
    const SourceFileMapping *conflict, *related;
    const AcquisitionUnitEntry *requester;
    SourceFileMapStatus status;
    bool ready;
} SourceFileMap;
/** Initialize an empty pure proposal owner, without filesystem access. */
void source_file_map_init(SourceFileMap *, SourceAllocator);
/** Release rows/keys before borrowed original observations expire. */
void source_file_map_destroy(SourceFileMap *);
/** Add one PRESENT captured regular file/directory and normalized absolute final
 * key. Repeated original keys require equal directory kind/file bytes and the
 * same output; distinct originals cannot share an output. Own both keys; borrow
 * content. Equal repeats allocate nothing. Failure keeps the visible prefix and
 * output, retains diagnostic witnesses, and poisons sealing. This is an exact
 * prepared-key relation, not path normalization, I/O or source transformation.
 * Time is O(M path comparisons + repeated bytes); space O(keys + one row).
 */
SourceFileMapStatus source_file_map_add(SourceFileMap *, const AcquisitionUnitEntry *,
    const char *destination, const SourceFileMapping **output);
/** Adapt plain immutable captured membership into an arena-owned descriptor,
 * then use normal insertion/diagnostics. Borrow original path/content through map
 * teardown; own both translated relation keys. Even an equal repeat can retain
 * one adapter allocation; no stack record is retained on success or failure.
 * Validation/allocation failures poison sealing and preserve output/prefix.
 * No I/O or source discovery; use only certified captured membership facts. */
SourceFileMapStatus source_file_map_observation(SourceFileMap *, const char *source,
    const SourceText *content, bool directory, const char *destination, const SourceFileMapping **output);
/** Admit every row of one ready explicit tree placement, retaining any failed
 * prefix as unsealable. Other unit/test/declared mappings may be joined later.
 */
SourceFileMapStatus source_file_map_tree(SourceFileMap *, const SourceTreePlacement *);
/** Freeze a complete successful relation, including an empty one, after checking
 * every pair for a final regular file occupying an ancestor directory position.
 * O(M squared) path comparisons, no allocation. Directory namespaces may contain
 * other entries; regular files may not. No output filesystem is accessed.
 */
SourceFileMapStatus source_file_map_seal(SourceFileMap *);
#endif
