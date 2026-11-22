/** @file
 * @brief Retire exact source-edit scratch before borrowed source owners expire.
 * [ARCH-03](../../../../spec/implementation/compiler.md#arch-03)
 * keeps complete and failed preparations under deterministic explicit ownership.
 */
#include "edits.h"
void source_text_edits_init(SourceTextEdits *edits, SourceAllocator allocator, const SourceText *original) {
    *edits = (SourceTextEdits){.original = original}; source_arena_init(&edits->arena, allocator);
}
void source_text_edits_destroy(SourceTextEdits *edits) {
    source_arena_destroy(&edits->arena); *edits = (SourceTextEdits){0};
}
