#include "semantic/internal.h"

static bool identical(const SemanticAccess *receiver, const SemanticSource *source) {
    return receiver != NULL && source->access != NULL
        && receiver->symbol == source->access->symbol
        && semantic_same_text(receiver->syntax->span, source->access->syntax->span);
}

bool semantic_transfer(SemanticProgram *program, SemanticType *type, bool reference,
    bool list, SemanticSource *source, const SemanticAccess *receiver,
    const SemanticSymbol *declaration, SemanticTransfer *transfer) {
    if (source->kind == AST_LITERAL_SOURCE) {
        if (reference || list || !type->abi.bindings[ABI_LITERAL].present) {
            return semantic_fail(program, source->site,
                "A literal requires owned scalar storage with the | capability", declaration);
        }
        source->type = type;
        *transfer = TRANSFER_LITERAL;
        return true;
    }
    bool continuation = source->kind == AST_CONTINUATION_SOURCE;
    if (source->type != type || source->list != list
        || (!continuation && source->reference != reference)
        || (continuation && (reference || list))) {
        return semantic_fail(program, source->site, "Source and destination types or storage forms differ", declaration);
    }
    if (identical(receiver, source)) {
        if (continuation) {
            return semantic_fail(program, source->site, "Continuation self-copy is forbidden", declaration);
        }
        *transfer = TRANSFER_NOOP;
        return true;
    }
    if (!reference && !type->cloneable) {
        return semantic_fail(program, source->site, "The owned source type does not support independent copying", declaration);
    }
    *transfer = reference ? TRANSFER_HANDLE : continuation ? TRANSFER_CONTINUATION : TRANSFER_CLONE;
    return true;
}
