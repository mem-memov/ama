#include "semantic/internal.h"

bool semantic_memory_failure(SemanticProgram *program) {
    program->status = SEMANTIC_OUT_OF_MEMORY;
    program->error = (Diagnostic){.code = DIAGNOSTIC_OUT_OF_MEMORY,
        .message = "Semantic analysis could not allocate storage"};
    return false;
}

bool semantic_fail(SemanticProgram *program, SourceSpan span, const char *message,
    const SemanticSymbol *related) {
    program->status = SEMANTIC_INVALID;
    program->error = (Diagnostic){.code = DIAGNOSTIC_TYPE_MISMATCH,
        .message = message, .primary = {.span = span}};
    if (related != NULL) {
        program->related[0] = (DiagnosticRelated){
            {.span = related->declaration->span},
            "related declaration", related->owner->module->source->logical_path};
        program->error.related = program->related;
        program->error.related_count = 1;
    }
    return false;
}
