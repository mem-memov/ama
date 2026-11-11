#include "semantic/internal.h"

static bool access_form(SemanticProgram *program, const AstAccess *syntax,
    const SemanticSymbol *symbol, bool writable, const SemanticSymbol *initializing) {
    bool selected_list = syntax->whole_list || syntax->index != NULL;
    if (syntax->reference != symbol->reference || selected_list != symbol->list) {
        return semantic_fail(program, syntax->span, "Access storage form does not match its declaration", symbol);
    }
    if (writable && symbol->role == SYMBOL_CONSTANT) {
        return semantic_fail(program, syntax->span, "Shared constant storage is immutable", symbol);
    }
    return semantic_visible(program, symbol, initializing, syntax->span);
}

bool semantic_condition(SemanticProgram *program, SemanticAccess *access,
    AbiOperation capability, SourceSpan site) {
    if (access->reference || access->list || !access->symbol->type->abi.bindings[capability].present) {
        return semantic_fail(program, site, capability == ABI_INDEX
            ? "The index must be an owned scalar with the [] capability"
            : "The condition must be an owned scalar with the ? capability", access->symbol);
    }
    return true;
}

SemanticAccess *semantic_access(SemanticProgram *program, SemanticType *type,
    const AstAccess *syntax, bool writable, const SemanticSymbol *initializing) {
    SemanticAccess *first = NULL;
    SemanticAccess **destination = &first;
    bool outer = true;
    for (const AstAccess *node = syntax; node != NULL; node = node->index) {
        SemanticSymbol *symbol = semantic_symbol(type, node->name);
        if (symbol == NULL) {
            semantic_fail(program, node->name, "The variable is not declared in this module", NULL);
            return NULL;
        }
        if (!access_form(program, node, symbol, outer && writable, initializing)) {
            return NULL;
        }
        SemanticAccess *access = semantic_allocate(program, sizeof(*access));
        if (access == NULL) {
            return NULL;
        }
        *access = (SemanticAccess){.syntax = node, .symbol = symbol,
            .reference = symbol->reference, .list = node->whole_list};
        if (!outer && !semantic_condition(program, access, ABI_INDEX, node->span)) {
            return NULL;
        }
        *destination = access;
        destination = &access->index;
        outer = false;
    }
    return first;
}
