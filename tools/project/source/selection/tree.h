/** @file
 * @brief Capture explicit filesystem SOURCE trees and project their original indexed module seeds.
 * [TOOL-01](../../../../spec/implementation/tools-and-conformance.md#tool-01)
 * distinguishes module files from directory selections;
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * requires the original selected catalog rather than inferred logical prefixes.
 * This component neither constructs a complete copy/removal set nor grants final
 * analysis-domain, ignored-version, dependency or publication authority.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires final whole-domain freshness and publication validation separately.
 */
#ifndef AMA_PROJECT_SOURCE_SELECTION_TREE_H
#define AMA_PROJECT_SOURCE_SELECTION_TREE_H
#include "../capture/capture.h"
/** Keep raw operand, physical read, original-unit mismatch and storage refusals separate. */
typedef enum { SOURCE_TREE_OK, SOURCE_TREE_INVALID, SOURCE_TREE_PHYSICAL,
    SOURCE_TREE_OUTSIDE, SOURCE_TREE_NOT_MODULE, SOURCE_TREE_FILES,
    SOURCE_TREE_CHANGED, SOURCE_TREE_MEMORY } SourceTreeStatus;
/** Own complete raw selected bytes/paths; borrow original catalog/unit handles.
 * valid means exact raw selection and coherent indexed observations only. Source
 * tests and complete units added outside an explicit directory, ignored/excluded
 * version review, graph/client safety and final freshness belong to outer planning.
 */
typedef struct {
    SourceArena arena;
    SourceManager files;
    const SourceRootCapture *capture;
    SourcePhysicalPath operand;
    SourcePhysicalIssue physical_issue;
    const AcquisitionUnitEntry *members;
    const SourceCapturedUnit *const *units;
    size_t count;
    const char *failed_path;
    SourceStatus file_status;
    SourceTreeStatus status;
    bool attempted, valid;
} SourceTreeSelection;
/** Begin owned scratch without filesystem reads. */
void source_tree_selection_init(SourceTreeSelection *, SourceAllocator);
/** Retire projected metadata before raw source bytes and borrowed catalog owners. */
void source_tree_selection_destroy(SourceTreeSelection *);
/** Resolve SOURCE under an explicit retained root. A file must be an exact
 * originally indexed main; a directory captures its entire physical tree. Bare
 * name+/ selection never selects its sibling main. Retain opaque bytes and empty
 * directories. Verify every originally indexed main/companion covered by the
 * physical selection; refuse changed observations without recapture fallback.
 * Borrow capture through result consumption. Single attempt; partial failure is
 * destructible but never valid. No tests, dependency closure, CMake or source
 * mutation occurs. [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * still requires complete-domain final freshness before publication.
 */
SourceTreeStatus source_tree_selection_read(SourceTreeSelection *, const SourceRootCapture *, const char *source);
#endif
