/** @file
 * @brief Share exact source-edit construction and graph-preservation checks for one rename plan.
 * [ARCH-06](../../../spec/implementation/compiler.md#arch-06) rejects identity changes rather than guessing intent.
 */
#ifndef AMA_EDITOR_RENAME_INTERNAL_H
#define AMA_EDITOR_RENAME_INTERNAL_H
#include "rename.h"
#include "../workspace/internal.h"
/** One exact original/candidate file pair, sorted by original physical path; optional edits provide prefix-coordinate deltas. Component contract: [ARCH-06](../../../spec/implementation/compiler.md#arch-06). */
struct EditorRenameSource {
    const ModuleSource *original;
    const ModuleSource *candidate;
    const EditorRenameFile *file;
};
/** Set an allocation-free retained failure; diagnostic source storage remains with the plan/workspace. Component contract: [ARCH-06](../../../spec/implementation/compiler.md#arch-06). */
bool editor_rename_fail(EditorRename *, DiagnosticCode, const char *, SourceSpan);
/** Validate/copy a complete base name through the same lexical primitives as the parser. Component contract: [ARCH-06](../../../spec/implementation/compiler.md#arch-06). */
bool editor_rename_name(EditorRename *, const unsigned char *, size_t);
/** Collect exact checked target occurrences and construct full-file replacements. Component contract: [ARCH-06](../../../spec/implementation/compiler.md#arch-06). */
bool editor_rename_files(EditorRename *);
/** Map an old original interval through this plan's exact nonoverlapping replacement ranges. Component contract: [ARCH-06](../../../spec/implementation/compiler.md#arch-06). */
bool editor_rename_span(const EditorRename *, SourceSpan, SourceSpan *);
/** Check the current catalog is exactly the original version, before installing candidate text. Component contract: [ARCH-06](../../../spec/implementation/compiler.md#arch-06). */
bool editor_rename_checkpoint(EditorRename *);
/** Install planned bytes into the candidate's owned catalog and run ordinary workspace validation. Component contract: [ARCH-06](../../../spec/implementation/compiler.md#arch-06). */
bool editor_rename_candidate(EditorRename *);
/** Compare all checked declarations, occurrences and call/binding metadata after exact coordinate mapping. Component contract: [ARCH-06](../../../spec/implementation/compiler.md#arch-06). */
bool editor_rename_identities(EditorRename *);
#endif
