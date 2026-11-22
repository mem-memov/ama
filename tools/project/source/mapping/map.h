/** @file
 * @brief Own one injective nominal/physical clone relation over the original catalog.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * forbids merging originals or implicitly cloning one original into conflicting
 * locations; equal bytes do not establish nominal type identity.
 */
#ifndef AMA_PROJECT_SOURCE_CLONE_MAP_H
#define AMA_PROJECT_SOURCE_CLONE_MAP_H
#include "../capture/capture.h"
/** Retain the first refusal; a failed prefix cannot be sealed as a complete map. */
typedef enum { SOURCE_CLONE_OK, SOURCE_CLONE_INVALID, SOURCE_CLONE_CONFLICT,
    SOURCE_CLONE_MEMORY } SourceCloneStatus;
/** One original catalog definition and owned final placement. Virtual type/state
 * generation belongs to the compiler, never to this source-side identity table.
 */
typedef struct SourceCloneMapping {
    struct SourceCloneMapping *next;
    const ModuleSource *original;
    const char *path, *identity;
} SourceCloneMapping;
/** Own rows/final strings, borrow the complete original capture. Prepared physical
 * keys and final identities come from outer placement validation. ready certifies
 * relation completeness/injectivity only, not destination existence, replacement,
 * context topology, package authority, edits or publication eligibility.
 */
typedef struct {
    SourceArena arena;
    const SourceRootCapture *capture;
    SourceCloneMapping *first;
    const SourceCloneMapping *conflict;
    const ModuleSource *requester;
    SourceCloneStatus status;
    bool ready;
} SourceCloneMap;
/** Begin owned relation metadata; original capture outlives all row use. */
void source_clone_map_init(SourceCloneMap *, SourceAllocator, const SourceRootCapture *);
/** Release complete/failed rows before the borrowed original catalog expires. */
void source_clone_map_destroy(SourceCloneMap *);
/** Propose one exact original handle and prepared absolute final physical main/
 * canonical final identity. Physical normalization/exact spelling is the caller's
 * prior obligation, not performed by this pure relation. Same proposal is an
 * allocation-free repeat. One original cannot acquire a second placement, and
 * distinct originals cannot share a final name or path. Copy strings and publish
 * output only on complete success; a failure poisons sealing but preserves owned
 * previous rows. After seal no further proposal, even a repeat, is allowed.
 * No filesystem reads, body analysis or mutation occurs.
 */
SourceCloneStatus source_clone_map_add(SourceCloneMap *, const ModuleSource *,
    const char *path, const char *identity, const SourceCloneMapping **output);
/** Seal an unfailed complete proposal set, including an empty relation. This
 * makes its invariant consumable; all outer placement/operation gates remain.
 */
SourceCloneStatus source_clone_map_seal(SourceCloneMap *);
#endif
