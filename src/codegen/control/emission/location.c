/**
 * @file
 * @brief Preserve each operation label, original source span and runtime trace selector.
 *
 * [STACK-01](../../../../spec/implementation/compiler.md#stack-01) keeps Ama
 * transfers as labels. [ARCH-03](../../../../spec/implementation/compiler.md#arch-03)
 * gives this writer only an output stream and resolved values; it never resolves names.
 */
#include "source/mapping/directive.h"
#include "emission.h"

bool codegen_control_location(FILE *output, size_t selector, SourceSpan site, bool record_operation) {
    if (fprintf(output, "ama_label_%zu: __attribute__((unused));\n", selector) < 0) { return false; }
    if (site.source != NULL && source_write_line_directive(output, site.source, site.start) != SOURCE_OK) { return false; }
    if (fputs("{ AmaStatus located=ama_program_cell(&ama_program,ama_current,&ama_cell);\n"
            "if(located.code!=AMA_OK) { ama_status=located; goto ama_project_finish; } }\n", output) == EOF) { return false; }
    return !record_operation || fprintf(output, "ama_cell->metadata.service.current_operation=UINT64_C(%zu);\n", selector) >= 0;
}
