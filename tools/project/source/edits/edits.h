/** @file
 * @brief Own exact parsed-span replacements and original-to-prepared occurrence provenance.
 * [TOOL-02](../../../../spec/implementation/tools-and-conformance.md#tool-02)
 * edits resolved operands without touching unrelated bytes;
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires prepared copies before source publication. Semantic span selection
 * remains the parser/resolver's responsibility, not this byte-edit component.
 */
#ifndef AMA_PROJECT_SOURCE_EDITS_H
#define AMA_PROJECT_SOURCE_EDITS_H
#include "source/memory/arena.h"
#include "source/text/source.h"
/** Sorted original half-open range and owned exact replacement bytes. */
typedef struct SourceTextEdit {
    struct SourceTextEdit *next;
    SourceSpan original;
    const unsigned char *bytes;
    size_t length;
} SourceTextEdit;
/** Keep invalid interfaces, overlapping edits, finite-size limits and allocation refusals distinct. */
typedef enum { SOURCE_EDITS_OK, SOURCE_EDITS_INVALID, SOURCE_EDITS_CONFLICT,
    SOURCE_EDITS_LIMIT, SOURCE_EDITS_MEMORY } SourceEditsStatus;
/** Own edits/replacements; borrow immutable input and, after application, the
 * output manager's result. Failure poisons application; no failed prefix is a
 * prepared source. This owner never mutates originals or the filesystem.
 */
typedef struct {
    SourceArena arena;
    const SourceText *original, *result;
    SourceTextEdit *first;
    const SourceTextEdit *conflict;
    SourceSpan failed;
    SourceEditsStatus status;
    bool attempted, valid;
} SourceTextEdits;
/** Borrow original through teardown; no source parsing or discovery occurs. */
void source_text_edits_init(SourceTextEdits *, SourceAllocator, const SourceText *original);
/** Release metadata before original and result source managers expire. */
void source_text_edits_destroy(SourceTextEdits *);
/** Admit only a valid exact-input span and borrowed replacement byte extent.
 * Empty replacement deletes; an empty span inserts. Insertion at a range boundary
 * is permitted (before replacement at its start), inside a replaced range is not.
 * Equal same-range bytes are allocation-free repeats; conflicting same-point
 * insertions refuse. Copy replacement bytes and publish only a complete row.
 * Inputs must be selected by an authoritative parsed operation: this component
 * grants no permission to edit arbitrary syntax. O(E) comparisons plus bytes,
 * O(replacement bytes + one row) new storage per distinct edit.
 */
SourceEditsStatus source_text_edits_add(SourceTextEdits *, SourceSpan,
    const unsigned char *, size_t);
/** Apply once into an explicit output manager/path, preserving every intervening
 * original byte, including NULs, invalid UTF-8 and line endings. Check sizes before
 * allocation and release temporary output bytes on every result. Output changes
 * only on success, which freezes the edits and borrows the manager-owned source.
 * No file I/O. O(input + output + E) time, O(output) temporary storage.
 */
SourceEditsStatus source_text_edits_apply(SourceTextEdits *, SourceManager *,
    const char *path, const SourceText **output);
/** Project an exact-input occurrence into the successfully prepared result.
 * Refuse endpoints strictly inside a replaced original range. For nonempty spans,
 * start uses right bias and end left bias at insertions, excluding outside text.
 * Empty anchors use right bias for both endpoints. Whole replacements and spans
 * containing edits are projectable. No allocation; output changes only on success.
 * O(E) time, O(1) scratch. Projection proves offsets under these edits, not that
 * the parser's final occurrence still denotes the same semantic entity.
 */
bool source_text_edits_project(const SourceTextEdits *, SourceSpan, SourceSpan *output);
#endif
