/** @file
 * @brief Plan exact identity-based Ama name edits and validate them before a client transaction.
 * [NAME-06](../../../spec/language/declarations.md#name-06) distinguishes scoped identities;
 * [SRC-03](../../../spec/language/source-and-names.md#src-03) preserves all unrelated bytes.
 */
#ifndef AMA_EDITOR_RENAME_H
#define AMA_EDITOR_RENAME_H
#include "../analysis/analysis.h"
/** Forward declaration of validation-owned original/candidate source coordinates. */
typedef struct EditorRenameSource EditorRenameSource;
/** One original complete-name range, excluding storage suffixes, ordered within its file. */
typedef struct EditorRenameEdit {
    struct EditorRenameEdit *next;
    SourceSpan original;
    /** Candidate boundaries include every earlier replacement in the same file. */
    size_t new_start;
    size_t new_end;
} EditorRenameEdit;
/** One versioned full-file replacement. Both old and new bytes remain borrowed until plan destruction;
 * original source/device/inode identify the expected document, not permission to overwrite a changed file.
 */
typedef struct EditorRenameFile {
    struct EditorRenameFile *next;
    const ModuleSource *original;
    const SourceText *replacement;
    EditorRenameEdit *first;
    EditorRenameEdit *last;
    const EditorRenameEdit **edits;
    size_t edit_count;
} EditorRenameFile;
/** A client-owned atomic document transaction receives all replacements once. It must verify expected
 * versions under exclusive edit authority and either apply all replacements or preserve every document.
 * Called synchronously by source tooling under the client's document ownership.
 */
typedef bool (*EditorRenameWriter)(void *, const EditorRenameFile *);
/** Own replacement bytes/indexes and candidate checking. Borrow the original complete workspace until
 * destruction. Validation diagnostics borrow the current candidate and expire on its next validation.
 */
typedef struct {
    SourceArena arena;
    SourceManager sources;
    const EditorAnalysis *workspace;
    const SemanticSymbol *target;
    SourceSpan name;
    EditorRenameFile *first;
    EditorRenameFile *last;
    EditorAnalysis candidate;
    const EditorRenameSource *source_index;
    size_t source_count;
    Diagnostic diagnostic;
    bool attempted;
    bool prepared;
    bool validated;
} EditorRename;
/** Initialize an empty plan and independent candidate checker with explicit allocator authority. */
void editor_rename_init(EditorRename *, SourceAllocator);
/** Destroy candidate/index/new-source storage before the borrowed original workspace may disappear. */
void editor_rename_destroy(EditorRename *);
/** Plan one variable/interface/constant/local rename at an indexed source spelling. Requires a complete
 * workspace analysis; new_name is one exact Ama base name. Comments, literals, paths and suffixes stay intact.
 * No filesystem writes occur. False retains a structured diagnostic and publishes prepared=false.
 */
bool editor_rename_prepare(EditorRename *, const EditorAnalysis *, const char *module, size_t offset,
    const unsigned char *new_name, size_t length);
/** Recheck original source inventory/native inputs, parse/resolve the exact candidate, and prove checked
 * declarations/references preserve their mapped identities. No edits are applied by validation.
 */
bool editor_rename_validate(EditorRename *);
/** Revalidate immediately before invoking the client's all-or-nothing versioned transaction.
 * A same-name no-op invokes no writer. Failed validation never invokes it; writer failure is reported.
 */
bool editor_rename_publish(EditorRename *, EditorRenameWriter, void *);
#endif
