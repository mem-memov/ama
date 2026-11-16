/** @file
 * @brief Own report strings and safely destructible failed copy prefixes.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * keeps recovery/report data independent of temporary source analysis owners.
 */
#include "internal.h"
/** Report initialization creates no project/semantic dependencies. */
void relocation_report_init(RelocationReport *report, SourceAllocator allocator) {
    *report = (RelocationReport){0};
    source_arena_init(&report->arena,allocator);
}
/** Report destruction never changes project files or recovery records. */
void relocation_report_destroy(RelocationReport *report) {
    source_arena_destroy(&report->arena);
    *report = (RelocationReport){0};
}
/** A failed copy prefix cannot be mistaken for a complete result. */
bool relocation_report_memory(RelocationReport *report) {
    report->status = RELOCATION_OUT_OF_MEMORY;
    return false;
}
