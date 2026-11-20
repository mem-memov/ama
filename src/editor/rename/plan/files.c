/** @file
 * @brief Collect identity-selected, source-ordered name edits and build exact independent file replacements.
 * [SRC-03](../../../../spec/language/source-and-names.md#src-03) preserves all non-name bytes;
 * [ARCH-06](../../../../spec/implementation/compiler.md#arch-06) uses checked finite byte concatenation.
 */
#include <stdint.h>
#include <string.h>
#include "../internal.h"
#include "source/io/buffer/buffer.h"
/** Source records are identified by their snapshot-owned text pointer, not a basename/path heuristic. Component contract: [SRC-03](../../../../spec/language/source-and-names.md#src-03), [ARCH-06](../../../../spec/implementation/compiler.md#arch-06). */
static const ModuleSource *source_file(const EditorRename *plan, const SourceText *text) {
    for (const ModuleSource *source = plan->workspace->frontend.project.catalog.first; source != NULL; source = source->next) {
        if (source->text == text) { return source; }
    }
    return NULL;
}
/** Append one unique occurrence already ordered/deduplicated by the checked editor index. Component contract: [SRC-03](../../../../spec/language/source-and-names.md#src-03), [ARCH-06](../../../../spec/implementation/compiler.md#arch-06). */
static bool occurrence(EditorRename *plan, SourceSpan span) {
    if (plan->last == NULL || plan->last->original->text != span.source) {
        const ModuleSource *source = source_file(plan,span.source);
        if (source == NULL) { return editor_rename_fail(plan,DIAGNOSTIC_TYPE_MISMATCH,"Rename source is outside the workspace",span); }
        EditorRenameFile *file = source_arena_allocate(&plan->arena,sizeof(*file));
        if (file == NULL) { return false; }
        *file = (EditorRenameFile){.original=source};
        if (plan->last == NULL) { plan->first = file; }
        else { plan->last->next = file; }
        plan->last = file;
    }
    EditorRenameEdit *edit = source_arena_allocate(&plan->arena,sizeof(*edit));
    if (edit == NULL) { return false; }
    *edit = (EditorRenameEdit){.original=span};
    if (plan->last->last == NULL) { plan->last->first = edit; }
    else { plan->last->last->next = edit; }
    plan->last->last = edit;
    if (plan->last->edit_count == SIZE_MAX) { return false; }
    plan->last->edit_count++;
    return true;
}
/** Copy untouched intervals verbatim and substitute only complete-name intervals; every failed buffer prefix retires. Component contract: [SRC-03](../../../../spec/language/source-and-names.md#src-03), [ARCH-06](../../../../spec/implementation/compiler.md#arch-06). */
static bool render(EditorRename *plan, EditorRenameFile *file) {
    SourceAllocator allocator = plan->arena.allocator;
    SourceBuffer buffer = {0};
    const unsigned char *original = source_bytes(file->original->text);
    size_t length = source_length(file->original->text), cursor = 0;
    SourceStatus status = SOURCE_OK;
    if (file->edit_count > SIZE_MAX/sizeof(*file->edits)) { return false; }
    file->edits = source_arena_allocate(&plan->arena,file->edit_count*sizeof(*file->edits));
    if (file->edits == NULL) { return false; }
    size_t ordinal = 0;
    for (EditorRenameEdit *edit = file->first; edit != NULL && status == SOURCE_OK; edit = edit->next) {
        SourceSpan span = edit->original;
        if (span.source != file->original->text || !source_span_valid(span) || span.start < cursor) {
            allocator.release(allocator.context,buffer.bytes);
            return editor_rename_fail(plan,DIAGNOSTIC_TYPE_MISMATCH,"Rename edits overlap or leave their source",span);
        }
        status = source_buffer_append(allocator,&buffer,original+cursor,span.start-cursor);
        edit->new_start = buffer.length;
        file->edits[ordinal++] = edit;
        if (status == SOURCE_OK) {
            status = source_buffer_append(allocator,&buffer,source_bytes(plan->name.source)+plan->name.start,plan->name.end-plan->name.start);
        }
        edit->new_end = buffer.length;
        cursor = span.end;
    }
    if (status == SOURCE_OK) { status = source_buffer_append(allocator,&buffer,original+cursor,length-cursor); }
    if (status == SOURCE_OK) {
        status = source_manager_add(&plan->sources,source_path(file->original->text),buffer.bytes,buffer.length,&file->replacement);
    }
    allocator.release(allocator.context,buffer.bytes);
    return status == SOURCE_OK;
}
/** Every target at an edited spelling must denote the one selected source
 * declaration. A shared label with another actual origin cannot be renamed as
 * a single declaration without changing that other binding.
 * [ARCH-06](../../../../spec/implementation/compiler.md#arch-06) preserves the existing one-declaration alpha-renaming contract.
 */
static bool selected(const EditorRename *plan, const EditorOccurrence *item) {
    return plan->target.static_symbol ? item->static_symbol == plan->target.static_symbol
        : plan->target.original && item->original == plan->target.original;
}

/** A same-name request is an explicit no-op. Equal text belonging to other identities never enters the edit set. Component contract: [SRC-03](../../../../spec/language/source-and-names.md#src-03), [ARCH-06](../../../../spec/implementation/compiler.md#arch-06). */
bool editor_rename_files(EditorRename *plan) {
    SourceSpan old = plan->target.definition;
    size_t length = old.end-old.start;
    if (length == plan->name.end-plan->name.start
        && memcmp(source_bytes(old.source)+old.start,source_bytes(plan->name.source)+plan->name.start,length) == 0) { return true; }
    for (const EditorOccurrence *item = plan->workspace->first; item != NULL; item = item->next) {
        bool matches = false, other = false;
        for (const EditorOccurrence *target = item; target; target = target->alternatives) {
            if (selected(plan,target)) { matches = true; } else { other = true; }
        }
        if (matches && other) {
            return editor_rename_fail(plan,DIAGNOSTIC_RENAME_CONFLICT,
                "Rename label has more than one original declaration",item->spelling);
        }
        if (matches && !occurrence(plan,item->spelling)) { goto failed; }
    }
    for (EditorRenameFile *file = plan->first; file != NULL; file = file->next) {
        if (!render(plan,file)) { goto failed; }
    }
    return true;
failed:
    return plan->diagnostic.message != NULL ? false
        : editor_rename_fail(plan,DIAGNOSTIC_OUT_OF_MEMORY,"Rename replacement size or allocation failed",old);
}
