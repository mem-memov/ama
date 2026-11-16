/**
 * @file
 * @brief Validate exact transfer form and mandatory copy capability before runtime alias handling.
 *
 * [COPY-06](../../../spec/language/values-and-references.md#copy-06)
 * Identical spelling never exempts owned copies; ordinary no-op identity is decided only after runtime selection/access checks.
 */
#include "semantic/internal.h"

/** Prove only a direct nonindexed location is the same resolved declaration for forbidden continuation self-copy. Indexed/reference aliases require runtime identity checks, never text equality.
 * Component contract: [COPY-06](../../../spec/language/values-and-references.md#copy-06). */
static bool same_direct_storage(const SemanticAccess *receiver, const SemanticSource *source) {
    return receiver != NULL && source->access != NULL
        && receiver->index == NULL && source->access->index == NULL
        && receiver->symbol == source->access->symbol;
}

bool semantic_transfer(SemanticProgram *program, SemanticType *type, bool reference,
    bool list, SemanticSource *source, const SemanticAccess *receiver,
    const SemanticSymbol *declaration, SemanticTransfer *transfer) {
    if (source->kind == AST_LITERAL_SOURCE) {
        source->type = type;
        if (reference || list) {
            return semantic_fail(program, source->site,
                "A literal requires owned scalar storage with the | capability", declaration);
        }
        if (!type->abi.bindings[ABI_LITERAL].present) {
            return semantic_capability_error(program, type, ABI_LITERAL, source->site, declaration,
                "The selected type does not support literal construction through |");
        }
        *transfer = TRANSFER_LITERAL;
        return true;
    }
    bool continuation = source->kind == AST_CONTINUATION_SOURCE;
    if (source->type != type || source->list != list
        || (!continuation && source->reference != reference)
        || (continuation && (reference || list))) {
        return semantic_incompatible(program, type, reference, list, source, declaration);
    }
    if (!reference && !type->cloneable) {
        return semantic_capability_error(program, type, ABI_CLONE, source->site, declaration,
            "The owned source type does not support independent copying");
    }
    if (continuation && same_direct_storage(receiver, source)) {
        return semantic_fail(program, source->site, "Continuation self-copy is forbidden", declaration);
    }
    *transfer = reference ? TRANSFER_HANDLE : continuation ? TRANSFER_CONTINUATION : TRANSFER_CLONE;
    return true;
}
