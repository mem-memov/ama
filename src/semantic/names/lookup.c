#include <string.h>
#include "semantic/internal.h"

SemanticType *semantic_type(const SemanticProgram *program, SourceSpan path) {
    size_t length = path.end - path.start;
    for (SemanticType *type = program->first; type != NULL; type = type->next) {
        const char *name = type->module->source->logical_path;
        if (strlen(name) == length && memcmp(name, source_bytes(path.source) + path.start, length) == 0) {
            return type;
        }
    }
    return NULL;
}

SemanticSymbol *semantic_symbol(const SemanticType *type, SourceSpan name) {
    for (SemanticSymbol *symbol = type->first; symbol != NULL; symbol = symbol->next) {
        if (semantic_same_text(symbol->declaration->as.declaration.name->name, name)) {
            return symbol;
        }
    }
    return NULL;
}
