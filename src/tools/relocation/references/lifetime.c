/** @file
 * @brief Preserve independent original analysis ownership and allocation-free failure records.
 * [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04)
 * requires failed preparation to leave the original project untouched.
 */
#include "internal.h"
/** Publish no borrowed preview until build; initialization performs no filesystem work. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
void relocation_references_init(RelocationReferences *references, SourceAllocator allocator) {
    *references = (RelocationReferences){0};
    source_arena_init(&references->arena,allocator);
    compiler_frontend_init(&references->frontend,allocator);
}
/** Semantic records borrow AST/source owners; every failed stage follows the same reverse order. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
void relocation_references_destroy(RelocationReferences *references) {
    compiler_frontend_destroy(&references->frontend);
    source_arena_destroy(&references->arena);
    *references = (RelocationReferences){0};
}
/** Fixed error prose and original source spans require no secondary diagnostic allocation. Component contract: [TOOL-04](../../../../spec/implementation/tools-and-conformance.md#tool-04). */
bool relocation_references_fail(RelocationReferences *references, RelocationStatus status, const char *message, SourceSpan site) {
    if (references->status == RELOCATION_OK) {
        references->status = status;
        references->error = (Diagnostic){.code=status == RELOCATION_OUT_OF_MEMORY ? DIAGNOSTIC_OUT_OF_MEMORY
            : DIAGNOSTIC_INVALID_PATH,.message=message,.primary={site,NULL}};
    }
    return false;
}
