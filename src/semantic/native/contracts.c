#include <assert.h>
#include "semantic/internal.h"

static bool companions(SemanticProgram *program, SemanticType *type) {
    const ModuleSource *source = type->module->source;
    const char *missing = source->native_header == NULL ? ".h"
        : source->native_c == NULL ? ".c" : source->native_abi == NULL ? ".abi" : NULL;
    if (missing == NULL) {
        return true;
    }
    SourceSpan selector = type->module->syntax.sections.implementation.marker->code;
    semantic_fail(program, selector, "A required native companion is missing", NULL);
    program->error.subject = source->logical_path;
    program->error.expected = missing;
    return false;
}

SemanticStatus semantic_native_authoring(SemanticProgram *program, const SemanticNativeAuthoring *authoring) {
    assert(program->names_valid && !program->native_attempted);
    program->native_attempted = true;
    for (SemanticType *type = program->first; type != NULL; type = type->next) {
        if (type->module->syntax.sections.kind != MODULE_NATIVE) {
            continue;
        }
        bool proposed = authoring != NULL && authoring->module == type->module;
        if (!proposed && !companions(program, type)) {
            return program->status;
        }
        const SourceText *abi = proposed ? authoring->abi : type->module->source->native_abi;
        ParseStatus status = parser_abi_parse(&type->abi, abi);
        if (status == PARSE_OUT_OF_MEMORY) {
            semantic_memory_failure(program);
        } else if (status != PARSE_OK) {
            program->status = SEMANTIC_INVALID;
            program->error = type->abi.error.diagnostic;
        }
        if (program->status != SEMANTIC_OK) {
            return program->status;
        }
    }
    program->native_valid = true;
    return SEMANTIC_OK;
}

SemanticStatus semantic_native(SemanticProgram *program) {
    return semantic_native_authoring(program, NULL);
}
