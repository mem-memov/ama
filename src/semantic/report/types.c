#include "semantic/internal.h"

static const char *source_role(bool reference, bool list) {
    static const char *const roles[] = {"source storage (owned scalar)", "source storage (reference scalar)",
        "source storage (owning list)", "source storage (reference list)"};
    return roles[(list ? 2 : 0) + reference];
}

static const char *destination_role(bool reference, bool list) {
    static const char *const roles[] = {"destination declaration (owned scalar)", "destination declaration (reference scalar)",
        "destination declaration (owning list)", "destination declaration (reference list)"};
    return roles[(list ? 2 : 0) + reference];
}

bool semantic_incompatible(SemanticProgram *program, SemanticType *type, bool reference,
    bool list, const SemanticSource *source, const SemanticSymbol *destination) {
    semantic_fail(program, source->site, "Source and destination types or storage forms differ", NULL);
    program->error.subject = source->type->module->source->logical_path;
    program->error.expected = type->module->source->logical_path;
    program->error.cause = source->type != type ? "The full module identities differ"
        : "The ownership or scalar/list forms differ";
    SourceSpan source_site = source->declaration == NULL
        ? (SourceSpan){source->type->module->source->text, 0, 0} : source->declaration->declaration->span;
    SourceSpan destination_site = destination == NULL
        ? (SourceSpan){type->module->source->text, 0, 0} : destination->declaration->span;
    program->related[0] = (DiagnosticRelated){{.span = source_site},
        source_role(source->reference, source->list), program->error.subject};
    program->related[1] = (DiagnosticRelated){{.span = destination_site},
        destination_role(reference, list), program->error.expected};
    program->error.related = program->related;
    program->error.related_count = 2;
    return false;
}

bool semantic_capability_error(SemanticProgram *program, SemanticType *type,
    AbiOperation operation, SourceSpan site, const SemanticSymbol *declaration, const char *message) {
    semantic_fail(program, site, message, declaration);
    program->error.subject = type->module->source->logical_path;
    SemanticType *contract = operation == ABI_CLONE && type->clone_seed != NULL ? type->clone_seed : type;
    const SourceText *abi = contract->abi.layout.source;
    SourceSpan contract_site = abi == NULL ? (SourceSpan){contract->module->source->text, 0, 0}
        : (SourceSpan){abi, source_length(abi), source_length(abi)};
    size_t index = program->error.related_count;
    program->related[index] = (DiagnosticRelated){{.span = contract_site},
        "type capability contract", contract->module->source->logical_path};
    program->error.related = program->related;
    program->error.related_count = index + 1;
    return false;
}
